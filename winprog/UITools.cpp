

#include "stdafx.h"
#include "uitools.h"
#include "blob.h"
#include "..\libcc\libcc\winapi.hpp"

using namespace LibCC;

namespace AutoPlacement
{
  // --------------------------------------------------------------------------------
  Position::Position() :
    percent(0),
    offset(0)
  {
  }

  Position::Position(const Position& rhs) :
    percent(rhs.percent),
    offset(rhs.offset)
  {
  }

  Position& Position::operator = (const Position& rhs)
  {
    percent = rhs.percent;
    offset = rhs.offset;
    return *this;
  }

  void Position::Clear()
  {
    percent = 0;
    offset = 0;
  }

  void Position::Add(const Position& x)
  {
    percent += x.percent;
    offset += x.offset;
  }

  void Position::Subtract(const Position& x)
  {
    percent -= x.percent;
    offset -= x.offset;
  }

  // this_position = parent_start + ((parent_end - parent_start) * this)
  void Position::TranslateRelativeTo(const Position& parent_start, const Position& parent_end)
  {
    Position temp = parent_end;
    temp.Subtract(parent_start);
    temp.Combine(*this);
    *this = temp;
    Add(parent_start);
  }

  void Position::Combine(const Position& rhs)
  {
    percent = static_cast<int>((static_cast<float>(rhs.percent) / 100) * percent);
    offset += rhs.offset;
  }

  // --------------------------------------------------------------------------------
  Identifier::Identifier()
  {
    Clear();
  }

  Identifier::Identifier(const Identifier& rhs) :
    m_Type(rhs.m_Type),
    m_NumericValue(rhs.m_NumericValue),
    m_Parts(rhs.m_Parts)
  {
  }

  Identifier& Identifier::operator =(const Identifier& rhs)
  {
    m_Parts = rhs.m_Parts;
    m_Type = rhs.m_Type;
    m_NumericValue = rhs.m_NumericValue;
    return *this;
  }

  void Identifier::Clear()
  {
    m_Type = Empty;
    m_NumericValue = 0;
    m_Parts.clear();
  }

  bool Identifier::Evaluate(Position& out, const UISettings& uiSettings, const VariableMap& vars, HDC dc, Orientation orientation, DebugLog& output) const
  {
    bool r = false;

    switch(m_Type)
    {
    case Identifier::Flexible:
      output.push_back(Message(_T("Unable to evaluate identifier (flexible).")));
      break;
    case Identifier::Empty:
      output.push_back(Message(_T("Unable to evaluate identifier (empty).")));
      break;
    case Identifier::Percent:
      out.percent = static_cast<int>(m_NumericValue);
      out.offset = 0;
      r = true;
      break;
    case Identifier::Pixels:
      out.percent = 0;
      out.offset = static_cast<int>(m_NumericValue);
      r = true;
      break;
    case Identifier::Dlus:
      {
        out.percent = 0;
        LONG baseUnits = GetDialogBaseUnits();
        if(orientation == Vertical)
        {
          LONG baseUnitY = HIWORD(baseUnits);
          out.offset = MulDiv(static_cast<int>(m_NumericValue), baseUnitY, 8);
        }
        else
        {
          LONG baseUnitX = LOWORD(baseUnits);
          out.offset = MulDiv(static_cast<int>(m_NumericValue), baseUnitX, 4);
        }

        r = true;
        break;
      }
    case Identifier::Points:
      {
        int index = (orientation == Vertical) ? LOGPIXELSY : LOGPIXELSX;
        out.percent = 0;
        out.offset = MulDiv(static_cast<int>(m_NumericValue), GetDeviceCaps(dc, index), 72);
        r = true;
        break;
      }
    case Identifier::Symbolic:
      {
        _tstring identifier = StringJoin(m_Parts.begin(), m_Parts.end(), _T("."));
        Identifier temp;
        int i;
        if(uiSettings.GetElement(i, identifier))
        {
          output.push_back(Message(Format("Symbolic identifier '%' unknown").s(identifier)));
        }
        else
        {
          temp.m_Type = Dlus;
          temp.m_NumericValue = static_cast<float>(i);
          r = temp.Evaluate(out, uiSettings, vars, dc, orientation, output);
        }
        break;
      }
    case Identifier::Variable:
      {
        if(m_Parts.size() != 1)
        {
          output.push_back(Message(_T("Invalid variable spec (part count != 1).")));
        }
        else
        {
          VariableMap::const_iterator it = vars.find(m_Parts[0]);
          if(it == vars.end())
          {
            output.push_back(Message(Format("Undefined variable '%'.").s(m_Parts[0])));
          }
          else
          {
            switch(it->second.m_Type)
            {
            case Identifier::Empty:
            case Identifier::Flexible:
            case Identifier::Symbolic:
            case Identifier::Variable:
              output.push_back(Message(Format("Variable '%' is not a value type.").s(m_Parts[0])));
              break;
            default:
              return it->second.Evaluate(out, uiSettings, vars, dc, orientation, output);
            }
          }
        }
        break;
      }
    default:
      output.push_back(Message(_T("Unknown units.")));
      break;
    }
    return r;
  }

  // --------------------------------------------------------------------------------
  Command::Command()
  {
    Clear();
  }

  Command::Command(const Command& rhs) :
    m_Assignment(rhs.m_Assignment),
    m_Distance(rhs.m_Distance),
    m_Orientation(rhs.m_Orientation),
    m_Children(rhs.m_Children),
    start(rhs.start),
    end(rhs.end)
  {
  }

  Command& Command::operator =(const Command& rhs)
  {
    m_Assignment = rhs.m_Assignment;
    m_Distance = rhs.m_Distance;
    m_Orientation = rhs.m_Orientation;
    m_Children = rhs.m_Children;
    start = rhs.start;
    end = rhs.end;
    return *this;
  }

  void Command::Clear()
  {
    m_Children.clear();
    m_Distance.Clear();
    m_Assignment.clear();
    m_Orientation = None;
  }

  // --------------------------------------------------------------------------------
  bool Manager::IsWhitespace(char x)
  {
    return (x == ' ') || (x == '\t');
  }
  bool Manager::IsNewline(char x)
  {
    return (x == '\r') || (x == '\n');
  }
  const char* Manager::GetIdentifierChars()
  {
    return "abcdefghijklmnopqrstuvwxyz0123456789_";
  }
  bool Manager::IsIdentifierChar(char x)
  {
    return (0 != strchr(GetIdentifierChars(), x));
  }

  // --------------------------------------------------------------------------------
  void Manager::SkipWhitespaceAndNewlines()
  {
    while(!m_script.eof())
    {
      if(!IsWhitespace(*m_script) && !IsNewline(*m_script))
      {
        break;
      }
      m_script.Advance();
    }
  }

  // --------------------------------------------------------------------------------
  // returns the char right after the parsed token.  assumes that we are on the start of a token (or whitespace before a token)
  bool Manager::ParseIdentifierPart(std::string& token)
  {
    SkipWhitespaceAndNewlines();
    token.clear();
    while(!m_script.eof())
    {
      if(!IsIdentifierChar(*m_script))
      {
        break;
      }
      token.push_back(*m_script);
      m_script.Advance();
    }
    return !token.empty();
  }

  // --------------------------------------------------------------------------------
  bool Manager::ParseIdentifier(const std::string& s_, Identifier& out)
  {
    bool r = true;
    std::string s;

    s = StringTrim(s_, " \t\r\n");

    // determine if it's a value like '100%' or '15px', or if it's a real identifier.
    if(s.length() && std::isdigit(s[0]))
    {
      // parse a numeric identifier.
      char* szSuffix;
      double val = strtod(s.c_str(), &szSuffix);
      out.m_NumericValue = static_cast<float>(val);

      std::string suffix = StringToLower(StringTrim(szSuffix, " \t\r\n"));
      if(suffix == "px" || suffix == "pixel" || suffix == "pixels" || suffix == "")
      {
        out.m_Type = Identifier::Pixels;
      }
      else if(suffix == "%" || suffix == "percent")
      {
        out.m_Type = Identifier::Percent;
      }
      else if(suffix == "pt" || suffix == "point" || suffix == "points")
      {
        out.m_Type = Identifier::Points;
      }
      else if(suffix == "dlu" || suffix == "dlus")
      {
        out.m_Type = Identifier::Dlus;
      }
      else
      {
        // error: unknown numeric prefix.  expected something like 'px', '%', 'points'
        Error(Format("Unknown units '%'").s(suffix));
        r = false;
      }
    }
    else
    {
      // parse a symbolic identifier.
      out.m_Type = Identifier::Symbolic;

      if(s == "flex" || s == "flexible")
      {
        out.m_Type = Identifier::Flexible;
      }
      else if(s.length() && s[0] == '<')
      {
        // variable.
        out.m_Type = Identifier::Variable;
        std::string::size_type n = s.find_first_of('>');
        if(n == std::string::npos)
        {
          Error(Format("Malformed variable name... missing '>' on identifier '%'").s(s));
        }
        else
        {
          out.m_Parts.clear();
          std::string trimmed = StringToLower(StringTrim(s.substr(1, n - 1), " \t\r\n"));
          
          if(std::string::npos != trimmed.find_first_not_of(GetIdentifierChars()))
          {
            // error - unrecognized character in variable
            Error(Format("Unrecognized character in variable '%'").s(trimmed));
            r = false;
          }
          else
          {
            out.m_Parts.push_back(StringConvert<TCHAR>(trimmed));
          }
        }
      }
      else
      {
        std::vector<std::string> Parts;
        StringSplitByString(s, ".", std::back_inserter(Parts));

        std::vector<std::string>::iterator itPart;
        for(itPart = Parts.begin(); itPart != Parts.end(); ++ itPart)
        {
          // trim
          std::string x = StringTrim(*itPart, " \t\r\n");
          std::string trimmed = StringToLower(x);

          // verify.  at this point we should have a part with nothing but identifier chars.
          if(std::string::npos != trimmed.find_first_not_of(GetIdentifierChars()))
          {
            // error - unrecognized character in identifier
            Error(Format("Unrecognized character in identifier '%'").s(trimmed));
            r = false;
          }
          else
          {
            out.m_Parts.push_back(StringConvert<TCHAR>(trimmed));
          }
        }
      }
    }
    return r;
  }

  // --------------------------------------------------------------------------------
  bool Manager::ParseIdentifierList(const std::string& sList, std::vector<Identifier>& out)
  {
    bool r = true;
    std::vector<std::string> Identifiers;
    StringSplitByString(sList, ",", std::back_inserter(Identifiers));

    std::vector<std::string>::iterator it;
    for(it = Identifiers.begin(); it != Identifiers.end(); ++ it)
    {
      Identifier x;
      if(ParseIdentifier(*it, x))
      {
        out.push_back(x);
      }
      else
      {
        r = false;
      }
    }
    return r;
  }

  // --------------------------------------------------------------------------------
  // a command is in the form [* assignment list ] [{ children; }] [distance];
  bool Manager::ParseCommand(Command& This)
  {
    bool r = false;
    // part . part . part (something) ;
    bool bErrors = false;
    SkipWhitespaceAndNewlines();
    bool bHasAssignments = false;

    if(*m_script != '*' && *m_script != '(' && *m_script != '{')
    {
      Error(Format("Unrecognized character in command.  Expected '(', '{', or '*'.  Make sure you prefix assignments with an asterisk.").s<10>(m_script.GetRawCursor()));
      bErrors = true;
    }
    else
    {
      if(*m_script == '*')
      {
        bHasAssignments = true;
        m_script.Advance();// ignore the asterisk; it's really not that important.
        SkipWhitespaceAndNewlines();
      }

      PreProcessor openbrace = m_script.FindChar('{');
      PreProcessor semicolon = m_script.FindChar(';');
      PreProcessor openparen = m_script.FindChar('(');
      PreProcessor closedparen = m_script.FindChar(')');

      if((openbrace < openparen) && (openbrace < semicolon))
      {
        // we have ourselves a parent node.  parse it out before continuing.
        PreProcessor temp = m_script;
        m_script = openbrace;
        m_script.Advance();
        ParseChildBlock(This);

        semicolon = m_script.FindChar(';');
        openparen = m_script.FindChar('(');
        closedparen = m_script.FindChar(')');

        m_script = temp;
      }

      if(!openparen.eof() && closedparen.eof())
      {
        Error(Format("Syntax error while parsing a command.  Closed parenthesis not found."));
        bErrors = true;
      }
      if(semicolon.eof())
      {
        Error(Format("Syntax error while parsing a command.  Semicolon not found."));
        bErrors = true;
      }

      std::string distance;

      if(openparen.eof() || (closedparen < openparen) || (semicolon < openparen))
      {
        // no parenthesis.  this means no distance.
        distance = "flex";
      }

      if(!bErrors)
      {
        if(bHasAssignments)
        {
          // extract tokens
          std::string assignment;

          PreProcessor endOfAssignments(min3(openbrace, openparen, semicolon));// in order of preference, 1) open brace, 2) open paren, 3) semicolon, whichever comes first & is not EOL.

          assignment = m_script.Substring(endOfAssignments);
          ParseIdentifierList(assignment, This.m_Assignment);
        }

        if(!distance.length() && !openparen.eof())
        {
          openparen.Advance();
          distance = openparen.Substring(closedparen);
        }
        r = ParseIdentifier(distance, This.m_Distance);

        m_script = semicolon;
        m_script.Advance();// advance past the semicolon.
      }
    }

    return r;
  }

  // --------------------------------------------------------------------------------
  // assumes x is after an open brace.
  // returns after the closing brace.
  // adds parsed nodes UNDER parent.
  bool Manager::ParseChildBlock(Command& parent)
  {
    bool r = true;
    while(1)
    {
      Command This;
      This.m_Orientation = parent.m_Orientation;
      SkipWhitespaceAndNewlines();
      if(m_script.eof())
      {
        // error: no closing brace found.
        Error(Format("No closing brace found on block."));
        r = false;
        break;
      }
      if(*m_script == '}')
      {
        m_script.Advance();
        // clean break;
        break;
      }
      if(!ParseCommand(This))
      {
        r = false;
        break;
      }
      parent.m_Children.push_back(This);
    }
    return r;
  }

  // --------------------------------------------------------------------------------
  void Manager::RegisterSymbol(const _tstring& name, HWND hwnd)
  {
    m_symbols[StringToLower(name)] = SymbolStorage(hwnd);
  }

  // --------------------------------------------------------------------------------
  bool Manager::RunString(const _tstring& script)
  {
    bool r = false;
    m_root.Clear();
    m_output.clear();

    m_script.SetString(script.c_str());

    std::string tok;
    while(1)
    {
      SkipWhitespaceAndNewlines();
      if(m_script.eof())
      {
        // end of file
        break;
      }
      Command This;
      ParseIdentifierPart(tok);
      if(m_script.eof())
      {
        Error(Format("Expected global 'vertical' or 'horizontal' block.  EOF found while parsing the block."));
        break;
      }
      SkipWhitespaceAndNewlines();
      if(*m_script != '{')
      {
        Error(Format("Expected '{', not found."));
        break;
      }

      m_script.Advance();// skip past the open brace.

      if(tok == "vertical")
      {
        This.m_Orientation = Vertical;
        ParseChildBlock(This);
        m_root.m_Children.push_back(This);
      }
      else if(tok == "horizontal")
      {
        This.m_Orientation = Horizontal;
        ParseChildBlock(This);
        m_root.m_Children.push_back(This);
      }
      else
      {
        Error(Format("Unrecognized characters at global scope (expected 'vertical' or 'horizontal')"));
      }

      // now skip past the semi-colon.  there should be NO cursor movement at this level.
      SkipWhitespaceAndNewlines();
      if(*m_script != ';')
      {
        Error(Format("Missing semi-colon after '%' block").s(tok));
      }
      if(!m_script.eof())
      {
        m_script.Advance();
      }
    }

    m_script.Clear();

    int indent = 0;

    // now interpret everything.
    r = Run();

    DumpNode(m_root, indent);
    DumpSymbols();

    return r;
  }

  // --------------------------------------------------------------------------------
  bool Manager::RunFile(const _tstring& fileName)
  {
    bool r = false;
    m_output.clear();
    HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(IsValidHandle(hFile))
    {
			LibCC::Blob<char> contents;
      DWORD size = GetFileSize(hFile, 0);
      if(contents.Alloc(size + 1))
      {
        DWORD br;
        ReadFile(hFile, contents.GetBuffer(), size, &br, 0);
        contents.GetBuffer()[size] = 0;
        if(br == size)
        {
          r = RunString(contents.GetBuffer());
        }
      }

      CloseHandle(hFile);
    }
    return r;
  }

  // --------------------------------------------------------------------------------
  bool Manager::Run(HDC dc)
  {
    Stack2<Command*> st;
    bool r = true;
    HDC dcScreen(0);

    m_cursor;

    if(!dc)
    {
      dcScreen = GetDC(0);
      dc = dcScreen;
    }

    m_root.start.percent = 0;
    m_root.start.offset = 0;
    m_root.end.percent = 100;
    m_root.end.offset = 0;

    //st.Push(&m_root);
    for(std::vector<Command>::iterator it = m_root.m_Children.begin(); it != m_root.m_Children.end(); ++it)
    {
      st.Push(&(*it));
      // all "root" blocks (vertical/horizontal) start at 0 and end at 100%
      it->start.percent = 0;
      it->start.offset = 0;
      it->end.percent = 100;
      it->end.offset = 0;
      ParseIdentifier("100%", it->m_Distance);
      r = RunCommand(st);
      st.Pop();
    }

    if(dcScreen)
    {
      ReleaseDC(0, dcScreen);
      dcScreen = 0;
    }

    return r;
  }

  // --------------------------------------------------------------------------------
  // runs the top of the stack.
  // assumes that the top of the stack has start/end cursor filled in already.
  bool Manager::RunCommand(Stack2<Command*>& st)
  {
    bool r = true;
    Command* node = st.Top();

    // make sure there is a maximum of ONE flexible container in this node.
    int nFlexible = 0;
    for(std::vector<Command>::iterator it = node->m_Children.begin(); it != node->m_Children.end(); ++it)
    {
      if(it->m_Distance.m_Type == Identifier::Flexible)
      {
        nFlexible ++;
      }
    }

    if(nFlexible > 1)
    {
      Error(Format("Multiple flexible containers found.  Only 1 is allowed per scope."));
      r = false;
    }
    else
    {
      void* it = 0;// make the previous one inaccessible here.
      /*
        The first goal here is to fill in start and end cursors for ALL child nodes.
        Since we can have a flexible node, start first by going from start -> end, and stop when we hit a flex
        node.  if we hit a flex node, then go from end->start until we hit it.  at that point
        all nodes should be filled in.
      */
      Position tempCursor = m_cursor;
      bool bFoundFlexNode = false;
      std::vector<Command>::iterator itNode;
      for(itNode = node->m_Children.begin(); itNode != node->m_Children.end(); ++itNode)
      {
        itNode->start = tempCursor;
        if(itNode->m_Distance.m_Type == Identifier::Flexible)
        {
          // found a flex node.  exit.
          bFoundFlexNode = true;
          break;
        }

        Position AbsoluteDistance;
        //if(!tempCursor.Add(m_variables, itNode->m_Distance, m_dc, itNode->m_Orientation, m_output))
        if(!itNode->m_Distance.Evaluate(AbsoluteDistance, m_settings, m_variables, m_dc, itNode->m_Orientation, m_output))
        {
          Error(Format("Error while advancing the cursor."));
          r = false;
          break;
        }
        tempCursor.Add(AbsoluteDistance);

        itNode->end = tempCursor;
      }

      if(r)
      {
        if(bFoundFlexNode)
        {
          // go bottom-up
          tempCursor.offset = 0;
          tempCursor.percent = 100;
          std::vector<Command>::reverse_iterator itNode;
          for(itNode = node->m_Children.rbegin(); itNode != node->m_Children.rend(); ++itNode)
          {
            itNode->end = tempCursor;
            if(itNode->m_Distance.m_Type == Identifier::Flexible)
            {
              // found our flex node.  exit.
              break;
            }

            Position AbsoluteDistance;
            //if(!tempCursor.Subtract(m_variables, itNode->m_Distance, m_dc, itNode->m_Orientation, m_output))
            if(!itNode->m_Distance.Evaluate(AbsoluteDistance, m_settings, m_variables, m_dc, itNode->m_Orientation, m_output))
            {
              Error(Format("Error while advancing the cursor."));
              r = false;
              break;
            }
            tempCursor.Subtract(AbsoluteDistance);

            itNode->start = tempCursor;
          }
        }

        if(r)
        {
          // ok now all children have start/end filled in.  do our own assignments
          if(!DoCommandAssignments(st))
          {
            Error(Format("Error while assigning."));
          }
          else
          {
            // run children
            for(itNode = node->m_Children.begin(); itNode != node->m_Children.end(); ++itNode)
            {
              st.Push(&(*itNode));
              if(!RunCommand(st))
              {
                r = false;
              }
              st.Pop();
            }
          }
        }
      }
    }
    return r;
  }

  // --------------------------------------------------------------------------------
  // does assignments for the top of the stack.
  bool Manager::DoCommandAssignments(const Stack2<Command*>& st)
  {
    bool r = true;
    Command* pc = st.Top();

    Stack2<Command*>::const_iterator it;

    // build our placement spec that we will assign to everything in the assignment list.
    Position start;
    Position end;
    {
      /*
        Right here, we have a hierarchy of regions.  Need to calculate this into a placement spec.
        We start at child and work our way out, adjusting the coords to the parent's each iteration.
        0%                                                   100%
        |-----------------------------------------------------|   (distance = 100%+0)
           15%+5                                       90%-4
             |-------------------------------------------|        (distance = 75%+1)
                 23%-6                       70%+13
                   |----------------------------|                 (distance = 47%+7)
                          35%+32    62%-64
                             |---------|                          (distance = 27%-32)

        Each child gets this sort of treatment to adjust it's coords to be at the paren't's scope:
        child_position = parent_start + (parent_distance * child_position)

        to re-word that, calculate the child position in terms of the parent, and offset it to where the parent's position is.

        so, starts for the above would look something like:
        x = 10% + (60% * 25%) = 10% + 15% = 25%
        40% + (50% * x) = 40% + (50% * 25%) = 40% + 12.5% = 52.5%
        0 + (100% * 52.5%) = ** 52.5% **
      */
      if(!st.size())
      {
        start.offset = 0;
        start.percent = 0;
        end.offset = 0;
        end.percent = 100;
      }
      else
      {
        // use the first one
        it = st.begin();
        Command* child = *it;

        start = child->start;
        end = child->end;

        ++ it;
        while(it != st.end())// iterate from child towards parent
        {
          // it points to parent.  child points to the ... child.
          Command* parent = *it;
          // start = parent_start + (parent_distance * start)
          start.TranslateRelativeTo(parent->start, parent->end);
          end.TranslateRelativeTo(parent->start, parent->end);
          ++ it;
        }
      }

    }

    // now do the assignments.
    for(std::vector<Identifier>::const_iterator it = pc->m_Assignment.begin(); it != pc->m_Assignment.end(); ++ it)
    {
      const Identifier& i(*it);
      // find the identifier in the list.
      // meaning of identifier parts at this point:
      // first: the actual symbol that's been registered
      // second: the member.  you could choose to only set the beginning cursor to a particular coordinate.
      _tstring Symbol;
      _tstring Member;

      if(i.m_Type != Identifier::Symbolic)
      {
        Error(Format("L-value expected. Can only assign values to symbols."));
        r = false;
        break;
      }

      switch(i.m_Parts.size())
      {
      case 1:
        Symbol = i.m_Parts[0];
        break;
      case 2:
        Symbol = i.m_Parts[0];
        Member = i.m_Parts[1];
        break;
      default:
        Error(Format("Wrong number of identifier parts."));
        r = false;
        break;
      }
      if(!r) break;

      // find the symbol.
      std::map<_tstring, SymbolStorage>::iterator itSymbol = m_symbols.find(Symbol);
      OptimizedPlacement* pPlacement = &(itSymbol->second.optimizedPlacement);
      if(itSymbol == m_symbols.end())
      {
        // error: symbol '' not found.
        Error(Format("Symbol '%' not registered.").s(Symbol));
        r = false;
        break;
      }

      if(!DoCommandAssignment(pc, start, end, pPlacement, Member))
      {
        // error propagating
        Error(Format("Error while assigning to '%'.").s(Symbol));
        r = false;
        break;
      }
    }

    return r;
  }

  // --------------------------------------------------------------------------------
  bool Manager::DoCommandAssignment(Command* pc, const Position& start, const Position& end, OptimizedPlacement* pPlacement, const _tstring& member)
  {
    bool r = true;

    // figure out what parts will be set.
    if(member.empty())
    {
      // figure it out based on orientation.
      if(pc->m_Orientation == Vertical)
      {
        pPlacement->m_VStart = start;
        pPlacement->m_VEnd = end;
      }
      else if(pc->m_Orientation == Horizontal)
      {
        pPlacement->m_HStart = start;
        pPlacement->m_HEnd = end;
      }
      else
      {
        Error(Format("Unknown orientation."));
        r = false;
      }
    }
    else
    {
      // here, they have specified which member to set. valid are left, top, right, bottom.  it must match the current orientation.
      if(pc->m_Orientation == Vertical)
      {
        if(member == "top")
        {
          pPlacement->m_VStart = start;
        }
        else if(member == "bottom")
        {
          pPlacement->m_VEnd = end;
        }
        else if(member == "left" || member == "right")
        {
          Error(Format("Mismatched orientation on member '%'.").s(member));
          r = false;
        }
        else
        {
          Error(Format("Unrecognized member '%'.").s(member));
          r = false;
        }
      }
      else if(pc->m_Orientation == Horizontal)
      {
        if(member == "left")
        {
          pPlacement->m_HStart = start;
        }
        else if(member == "right")
        {
          pPlacement->m_HEnd = end;
        }
        else if(member == "top" || member == "bottom")
        {
          Error(Format("Mismatched orientation on member '%'.").s(member));
          r = false;
        }
        else
        {
          Error(Format("Unrecognized member '%'.").s(member));
          r = false;
        }
      }
      else
      {
        Error(Format("Unknown orientation."));
        r = false;
      }
    }

    return r;
  }

  // --------------------------------------------------------------------------------
  void Manager::Warning(const _tstring& msg)
  {
    m_output.push_back(Message(msg, Message::Warning, m_script.GetLine(), m_script.GetColumn()));
    OutputDebugString(Format("RealTime: %|").s(m_output.back().ToString()));
  }

  // --------------------------------------------------------------------------------
  void Manager::Error(const _tstring& msg)
  {
    m_output.push_back(Message(msg, Message::Error, m_script.GetLine(), m_script.GetColumn()));
    OutputDebugString(Format("RealTime: %|").s(m_output.back().ToString()));
  }

  // --------------------------------------------------------------------------------
  void Manager::DumpOutput() const
  {
    std::vector<Message>::const_iterator it;
    for(it = m_output.begin(); it != m_output.end(); ++ it)
    {
      OutputDebugString(Format("%|").s(it->ToString()));
    }
  }

  // --------------------------------------------------------------------------------
  bool Manager::OnSize() const
  {
    std::map<_tstring, SymbolStorage>::const_iterator it;
    for(it = m_symbols.begin(); it != m_symbols.end(); ++it)
    {
      const SymbolStorage& ss(it->second);
      ss.optimizedPlacement.OnSize(ss.hwnd);
    }
    return true;
  }

  // --------------------------------------------------------------------------------
  void Manager::DumpNode(const Command& n, int& indent)
  {
    OutputDebugString(Format("%").c('\t', indent));
    OutputDebugString(n.ToString(indent).c_str());
    OutputDebugString(Format("|%{|").c('\t', indent));
    indent ++;
    std::vector<Command>::const_iterator it;
    for(it = n.m_Children.begin(); it != n.m_Children.end(); ++ it)
    {
      DumpNode(*it, indent);
    }
    indent --;
    OutputDebugString(Format("%};|").c('\t', indent));
  }

  // --------------------------------------------------------------------------------
  void Manager::DumpSymbols() const
  {
    std::map<_tstring, SymbolStorage>::const_iterator it;
    for(it = m_symbols.begin(); it != m_symbols.end(); ++it)
    {
      OutputDebugString(Format("[%]|{|%}||").s(it->first).s(it->second.ToString(1)));
    }
  }

  // --------------------------------------------------------------------------------
  _tstring Command::ToString(int indent) const
  {
    // construct a string of assignments
    std::vector<_tstring> assignments;

    std::vector<Identifier>::const_iterator itIdentifier;
    for(itIdentifier = m_Assignment.begin(); itIdentifier != m_Assignment.end(); ++ itIdentifier)
    {
      assignments.push_back(itIdentifier->ToString());
    }

    _tstring strAssignments = LibCC::StringJoin(assignments.begin(), assignments.end(), _T(", "));

    // orientation to string
    _tstring orientation;
    switch(m_Orientation)
    {
    case Vertical:
      orientation = _T("Vertical");
      break;
    case Horizontal:
      orientation = _T("Horizontal");
      break;
    case None:
      orientation = _T("None");
      break;
    default:
      orientation = _T("unknown");
      break;
    }

    return LibCC::Format("orient[%]|%assign[%]|%distan[% start=% end=%]")
      .s(orientation)
      .c('\t', indent)
      .s(strAssignments)
      .c('\t', indent)
      .s(m_Distance.ToString())
      .s(start.ToString())
      .s(end.ToString());
  }

}
