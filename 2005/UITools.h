/*
  January 25, 2005 carlc

  TODO:
  - Layout symbols
  - Warning on untouched members

  - Allow multiple flex-nodes and give them equal spacing

  x Optimize placement structs
  x vertical % bug
  x Comment parsing
*/

#pragma once

#include <vector>
#include <map>
#include <string>
#include <iterator>
#include <cctype>
#include <stack>
#include <algorithm>
#include "stack2.h"


using namespace LibCC;

//--------------------------------------------------------------------------------
// allows nesting.
namespace AutoPlacement
{
  template<typename T>
  const T& min2(const T& lhs, const T& rhs)
  {
    return lhs < rhs ? lhs : rhs;
  }
  template<typename T>
  const T& min3(const T& a, const T& b, const T& c)
  {
    return min2(min2(a, b), c);
  }

  // --------------------------------------------------------------------------------
  enum Orientation
  {
    None,
    Vertical,
    Horizontal
  };

  // --------------------------------------------------------------------------------
  class Message
  {
  public:
    enum Severity
    {
      Debug,
      Warning,
      Error
    };

    Message() :
      line(0),
      column(0),
      severity(Error)
    {
    }

    Message(const _tstring& message_, Severity severity_ = Error, long line_ = 0, long column_ = 0) :
      message(message_),
      severity(severity_),
      line(line_),
      column(column_)
    {
    }

    Message(const Message& r) :
      message(r.message),
      severity(r.severity),
      line(r.line),
      column(r.column)
    {
    }

    Message& operator =(const Message& r)
    {
      message = r.message;
      severity = r.severity;
      line = r.line;
      column = r.column;
      return *this;
    }

    _tstring ToString() const
    {
      _tstring sev;
      switch(severity)
      {
      case Message::Debug:
        sev = _T("Debug");
        break;
      case Message::Warning:
        sev = _T("Warning");
        break;
      case Message::Error:
        sev = _T("Error");
        break;
      }
      return LibCC::Format("(%, %) %: %").l(line).l(column).s(sev).s(message);
    }

    long line;
    long column;
    _tstring message;
    Severity severity;
  };

  // --------------------------------------------------------------------------------
  // units are always in pixels.
  class Position
  {
  public:
    Position();
    Position(const Position& rhs);
    Position(int per, int off) :
      percent(per),
      offset(off)
    {
    }
    Position& operator = (const Position& rhs);

    void Clear();

    void Add(const Position& x);
    void Subtract(const Position& x);
    /*
      Sort of hard to describe what "translate" does.  It's an arithmetical operation...
      basically the idea is to restate a position, relative to a different container.  consider the example figures:
          0%                                                   100%
      A:  |-----------------------------------------------------|   (distance = 100%+0)
             15%+5                                       90%-4
      B:       |-------------------------------------------|        (distance = 75%+1)
                   23%-6                       70%+13
      C:             |----------------------------|                 (distance = 47%+7)

      Right now, (23%-6) is relative to {B}.  In order to make it relative to {A}, we need to to a translation,
      which involves knowing the current parent (in this case, {B}).
    */
    void TranslateRelativeTo(const Position& parent_start, const Position& parent_end);

    /*
      Combining is used by TranslateRelativeTo().  It's sort of like a multiply, but i don't like to
      use that term because it's:
      percent *= rhs.percent;
      offset += rhs.offset;
    */
    void Combine(const Position& rhs);

    int percent;
    int offset;

    _tstring ToString() const
    {
      return LibCC::Format("(%^% + %)").l(percent).l(offset);
    }
  };

  // --------------------------------------------------------------------------------
  // most of this gathered from
  // "Design Specifications and Guidelines - Visual Design"
  // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnwue/html/ch14e.asp
  // all items are in DLU's
  class UISettings
  {
  public:
    UISettings()
    {
    }
    
    void LoadDefaults()
    {
      ControlSpacingRelated = 4;
      ControlSpacingUnrelated = 7;
      LabelHeight = 8;
      ParagraphSpacing = 7;
      ClarityIndent = 10;
      LabelSpacingFromAssociatedControlTop = 3;

      // dialog
      DialogMarginLeft = 7;
      DialogMarginRight = 7;
      DialogMarginTop = 7;
      DialogMarginBottom = 7;
      DialogWideMarginLeft = 21;
      DialogWideMarginRight = 21;

      // group
      GroupMarginLeft = 9;
      GroupMarginTop = 11;
      GroupMarginRight = 9;
      GroupMarginBottom = 7;

      // controls
      ButtonWidth = 50;
      ButtonHeight = 14;
      CheckboxHeight = 10;
      ComboboxHeight = 14;
      RadioButtonHeight = 10;
      EditBoxHeight = 14;
    }

    bool GetElement(int& out, const _tstring& s) const
    {
      if(StringEqualsI(s, "spacing.relatedcontrol") || StringEqualsI(s, "spacing"))
      {
        out = ControlSpacingRelated;
      }
      else if(StringEqualsI(s, "spacing.unrelatedcontrol") || StringEqualsI(s, "spacing.unrelated"))
      {
        out = ControlSpacingUnrelated;
      }
      else if(StringEqualsI(s, "spacing.clarityindent") || StringEqualsI(s, "spacing.clarityindent.left"))
      {
        out = ClarityIndent;
      }
      else if(StringEqualsI(s, "margin.left"))
      {
        out = DialogMarginLeft;
      }
      else if(StringEqualsI(s, "margin.right"))
      {
        out = DialogMarginRight;
      }
      else if(StringEqualsI(s, "margin.top"))
      {
        out = DialogMarginTop;
      }
      else if(StringEqualsI(s, "margin.bottom"))
      {
        out = DialogMarginBottom;
      }
      else if(StringEqualsI(s, "widemargin.left"))
      {
        out = DialogWideMarginLeft;
      }
      else if(StringEqualsI(s, "widemargin.right"))
      {
        out = DialogWideMarginRight;
      }
      else if(StringEqualsI(s, "label.height"))
      {
        out = LabelHeight;
      }
      else if(StringEqualsI(s, "label.paragraphspacing"))
      {
        out = ParagraphSpacing;
      }
      else if(StringEqualsI(s, "label.offset.top") || StringEqualsI(s, "label.offset"))
      {
        out = LabelSpacingFromAssociatedControlTop;
      }
      else if(StringEqualsI(s, "group.margin.left"))
      {
        out = GroupMarginLeft;
      }
      else if(StringEqualsI(s, "group.margin.top"))
      {
        out = GroupMarginTop;
      }
      else if(StringEqualsI(s, "group.margin.right"))
      {
        out = GroupMarginRight;
      }
      else if(StringEqualsI(s, "group.margin.bottom"))
      {
        out = GroupMarginBottom;
      }
      else if(StringEqualsI(s, "button.width"))
      {
        out = ButtonWidth;
      }
      else if(StringEqualsI(s, "button.height"))
      {
        out = ButtonHeight;
      }
      else if(StringEqualsI(s, "checkbox.height"))
      {
        out = CheckboxHeight;
      }
      else if(StringEqualsI(s, "combobox.height"))
      {
        out = ComboboxHeight;
      }
      else if(StringEqualsI(s, "radio.height"))
      {
        out = RadioButtonHeight;
      }
      else if(StringEqualsI(s, "editbox.height") || StringEqualsI(s, "edit.height"))
      {
        out = EditBoxHeight;
      }
      else
      {
        return false;
      }
      return true;
    }

    // general layout
    int ControlSpacingRelated;// 4 dlu
    int ControlSpacingUnrelated;// 7 dlu
    int LabelHeight;// 8 dlu per line
    int ParagraphSpacing;// 7 dlu
    int ClarityIndent;// 10 dlu  -- when there is a label and some controls below, indented.
    int LabelSpacingFromAssociatedControlTop;//3 dlu -- a label with an edit box, for instance.. gets placed 3 dlu's below the top of the edit

    // dialog
    int DialogMarginLeft;// 7 dlu
    int DialogMarginRight;// 7 dlu
    int DialogMarginTop;// 7 dlu
    int DialogMarginBottom;// 7 dlu
    int DialogWideMarginLeft;// 21 dlu
    int DialogWideMarginRight;// 21 dlu

    // group
    int GroupMarginLeft;// 9 dlu
    int GroupMarginTop;// 11 dlu
    int GroupMarginRight;// 9 dlu
    int GroupMarginBottom;// 7 dlu

    // controls
    int ButtonWidth;// 50 dlu
    int ButtonHeight;// 14 dlu
    int CheckboxHeight;// 10 dlu
    int ComboboxHeight;// 14 dlu
    int RadioButtonHeight;// 10 dlu
    int EditBoxHeight;// 14 dlu
  };

  // --------------------------------------------------------------------------------
  /*
    An identifier is a symbolic or numeric value, with units.  after it's "translated" into being ALWAYS numeric,
    it's converted to a Position, in pixels
  */
  class Identifier
  {
  public:
    Identifier();
    Identifier(const Identifier& rhs);
    Identifier& operator =(const Identifier& rhs);
    void Clear();

    enum Type
    {
      Empty,
      Flexible,
      Symbolic,// uses m_Parts
      Variable,
      Percent,
      Pixels,
      Dlus,
      Points
    };

    Type m_Type;
    float m_NumericValue;
    std::vector<_tstring> m_Parts;

    typedef std::map<_tstring, Identifier> VariableMap;
    typedef std::vector<Message> DebugLog;

    bool Evaluate(Position& out, const UISettings& uiSettings, const VariableMap& vars, HDC dc, Orientation orientation, DebugLog& dbg) const;

    // (assignment [type])
    _tstring ToString() const
    {
      // construct a string that represents assignments;
      _tstring assignments = LibCC::StringJoin(m_Parts.begin(), m_Parts.end(), _T("."));

      // type->string
      _tstring type;
      switch(m_Type)
      {
      case Empty:
        type = "emp"; break;
      case Symbolic:
        type = "sym"; break;
      case Percent:
        type = "%"; break;
      case Pixels:
        type = "px"; break;
      case Variable:
        type = "var"; break;
      case Dlus:
        type = "dlu"; break;
      case Points:
        type = "pt"; break;
      case Flexible:
        type = "flex"; break;
      default:
        type = "def"; break;
      }

      return LibCC::Format("% [%%]").s(assignments).f<2>(m_NumericValue).s(type);
    }
  };

  // --------------------------------------------------------------------------------
  class OptimizedPlacement
  {
  public:
    Position m_HStart;
    Position m_HEnd;
    Position m_VStart;
    Position m_VEnd;

    int nHStart;
    int nHEnd;
    int nVStart;
    int nVEnd;

    OptimizedPlacement() :
      nHStart(0),
      nHEnd(0),
      nVStart(0),
      nVEnd(0)
    {
    } 
    OptimizedPlacement(const OptimizedPlacement& rhs) :
      m_HStart(rhs.m_HStart),
      m_HEnd(rhs.m_HEnd),
      m_VStart(rhs.m_VStart),
      m_VEnd(rhs.m_VEnd),
      nHStart(rhs.nHStart),
      nHEnd(rhs.nHEnd),
      nVStart(rhs.nVStart),
      nVEnd(rhs.nVEnd)
    {
    }
    OptimizedPlacement& operator =(const OptimizedPlacement& rhs)
    {
      m_HStart = rhs.m_HStart;
      m_HEnd = rhs.m_HEnd;
      m_VStart = rhs.m_VStart;
      m_VEnd = rhs.m_VEnd;
      nHStart = rhs.nHStart;
      nHEnd = rhs.nHEnd;
      nVStart = rhs.nVStart;
      nVEnd = rhs.nVEnd;
      return *this;
    }
    // resizes p to its parent, based on the placement stuff.
    void OnSize(HWND hWnd) const
    {
      CRect rcClient;// current container's coordinates, based on the parent window.
      GetClientRect(GetParent(hWnd), rcClient);
      OnSize(hWnd, rcClient);
    }
    void OnSize(HWND hWnd, const CRect& rcClient) const
    {
      CRect rcNew;
      rcNew.left = GetAdjustedCoord(m_HStart.percent, m_HStart.offset, rcClient.left, rcClient.right);
      rcNew.right = GetAdjustedCoord(m_HEnd.percent, m_HEnd.offset, rcClient.left, rcClient.right);
      rcNew.top = GetAdjustedCoord(m_VStart.percent, m_VStart.offset, rcClient.top, rcClient.bottom);
      rcNew.bottom = GetAdjustedCoord(m_VEnd.percent, m_VEnd.offset, rcClient.top, rcClient.bottom);
      MoveWindow(hWnd, rcNew.left, rcNew.top, rcNew.Width(), rcNew.Height(), TRUE);
    }

    _tstring ToString(int indent) const
    {
      return _T("");
    }

  private:
    static int GetAdjustedCoord(int percent, int offset, int originalStart, int originalEnd)
    {
      return originalStart + offset + MulDiv(originalEnd - originalStart, percent, 100);
    }
  };


  // --------------------------------------------------------------------------------
  class Command
  {
  public:
    Command();
    Command(const Command& rhs);
    Command& operator =(const Command& rhs);

    void Clear();

    Identifier m_Distance;
    std::vector<Identifier> m_Assignment;

    // if it's a parent node:
    Orientation m_Orientation;
    std::vector<Command> m_Children;

    // used by the script runner
    Position start;
    Position end;

    _tstring ToString(int indent) const;
  };

  // --------------------------------------------------------------------------------
  class PreProcessor
  {
  public:
    PreProcessor() :
      start(0),
      cursor(0),
      line(0),
      column(0)
    {
    }
    PreProcessor(const PreProcessor& rhs) :
      start(rhs.start),
      cursor(rhs.cursor),
      line(rhs.line),
      column(rhs.column)
    {
    }
    PreProcessor& operator =(const PreProcessor& rhs)
    {
      start = rhs.start;
      cursor = rhs.cursor;
      line = rhs.line;
      column = rhs.column;
      return *this;
    }
    void SetString(const char* s)
    {
      start = s;
      cursor = s;
      line = 1;
      column = 1;
      AdvancePastComments();
    }
    void Clear()
    {
      start = 0;
      cursor = 0;
      line = 0;
      column = 0;
    }
    const char* GetRawCursor() const { return cursor; }
    const char* GetRawStart() const { return start; }
    bool eof() const
    {
      return (cursor == 0) || (*cursor == 0); 
    }
    
    bool operator < (const PreProcessor& rhs) const { return (rhs.start == start) && (cursor < rhs.cursor); }
    bool operator > (const PreProcessor& rhs) const { return (rhs.start == start) && (cursor > rhs.cursor); }
    bool operator <= (const PreProcessor& rhs) const { return (rhs.start == start) && (cursor <= rhs.cursor); }
    bool operator >= (const PreProcessor& rhs) const { return (rhs.start == start) && (cursor >= rhs.cursor); }
    bool operator == (const PreProcessor& rhs) const { return (rhs.start == start) && (cursor == rhs.cursor); }
    bool operator != (const PreProcessor& rhs) const { return (rhs.start == start) && (cursor != rhs.cursor); }

    char operator *() const { return cursor ? *cursor : 0; }
    char operator [](size_t i) const
    {
      if(!cursor)
      {
        return 0;
      }
      if(i > strlen(cursor))
      {
        return 0;
      }
      return cursor[i];
    }
    long GetLine() const { return line; }
    long GetColumn() const { return column; }
    PreProcessor FindChar(char c) const
    {
      PreProcessor temp(*this);
      while(!temp.eof())
      {
        if(temp[0] == c)
        {
          return temp;
        }
        temp.Advance();
      }
      return PreProcessor();
    }
    std::string Substring(const PreProcessor& end) const
    {
      if(end.start != start) return "";
      PreProcessor temp(*this);
      std::string r;
      r.reserve(end.cursor - temp.cursor + 1);

      while(!temp.eof() && (end.cursor > temp.cursor))
      {
        r.push_back(*temp);
        temp.Advance();
      }

      return r;
    }
    const char* Advance()
    {
      InternalAdvance();
      AdvancePastComments();
      return cursor;
    }
  private:
    void AdvancePastComments()
    {
      while(1)
      {
        if(0 == strncmp(cursor, "//", 2))
        {
          // advance until a newline.
          long oldLine = line;
          while(*cursor && (line == oldLine))
          {
            InternalAdvance();
          }
        }
        else if(0 == strncmp(cursor, "/*", 2))
        {
          while(0 != strncmp(cursor, "*/", 2))
          {
            InternalAdvance();
          }
          // go past the terminator
          InternalAdvance();
          InternalAdvance();
        }
        else
        {
          break;
        }
      }
    }
    // adjusts line & column while advancing 1 char.
    void InternalAdvance()
    {
      // adjust new line
      if(*cursor == '\n')
      {
        line ++;
        column = 0;
      }

      if(*cursor)
      {
        cursor ++;
        column ++;
      }

      // skip \r characters altogether
      if(*cursor == '\r')
      {
        cursor ++;
      }
    }

    const char* start;
    const char* cursor;
    long line;
    long column;
  };

  // --------------------------------------------------------------------------------
  class Manager
  {
  public:
    Manager()
    {
      m_settings.LoadDefaults();
    }

    void RegisterSymbol(const _tstring& name, HWND hwnd);
    void RegisterVariable(const _tstring& name, int pixels)
    {
      Identifier i;
      i.m_NumericValue = static_cast<float>(pixels);
      i.m_Type = Identifier::Pixels;
      _tstring nameLower(name);
      LibCC::StringToLower(nameLower);
      m_variables[nameLower] = i;
    }
    void RegisterVariable(const _tstring& name, const _tstring& spec)
    {
      Identifier i;
      if(ParseIdentifier(spec, i))
      {
        _tstring nameLower(name);
        LibCC::StringToLower(nameLower);
        m_variables[nameLower] = i;
      }
    }
    bool RunString(const _tstring& script);
    bool RunFile(const _tstring& fileName);

    bool OnSize() const;

    void DumpOutput() const;
    //void DumpStack(const Stack2<Command*>& st) const;
    static void DumpNode(const Command&, int& indent);
    void DumpSymbols() const;

    std::vector<Message> m_output;
    UISettings m_settings;

  private:
    void SkipWhitespaceAndNewlines();
    bool ParseIdentifierPart(std::string& token);
    bool ParseIdentifier(const std::string& s, Identifier& out);
    bool ParseIdentifierList(const std::string& sList, std::vector<Identifier>& out);
    bool ParseCommand(Command& This);
    bool ParseChildBlock(Command& parent);

    bool Run(HDC dc = 0);
    bool RunCommand(Stack2<Command*>& st);
    bool DoCommandAssignments(const Stack2<Command*>& st);
    bool DoCommandAssignment(Command* pc, const Position& starts, const Position& ends, OptimizedPlacement* pPlacement, const _tstring& member);

    void Warning(const _tstring& msg);
    void Error(const _tstring& msg);

    static const char* GetIdentifierChars();
    static bool IsWhitespace(char x);
    static bool IsNewline(char x);
    static bool IsIdentifierChar(char x);

    HDC m_dc;
    Position m_cursor;
    Command m_root;

    // line numbers.
    PreProcessor m_script;

    class SymbolStorage
    {
    public:
      SymbolStorage(HWND h = 0) : hwnd(h) {}
      SymbolStorage(const SymbolStorage& rhs) :
        hwnd(rhs.hwnd),
        optimizedPlacement(rhs.optimizedPlacement)
      { }
      SymbolStorage& operator =(const SymbolStorage& rhs)
      {
        hwnd = rhs.hwnd;
        optimizedPlacement = rhs.optimizedPlacement;
        return *this;
      }
      _tstring ToString(int indent) const
      {
        return optimizedPlacement.ToString(indent);
      }

      OptimizedPlacement optimizedPlacement;
      HWND hwnd;
    };

    std::map<_tstring, SymbolStorage> m_symbols;
    std::map<_tstring, Identifier> m_variables;
  };
}

