X11 color database
http://www.newcottage.com/index.php?section=lab&subsection=lab/x11_colors

TODO:
- finish UI design
- start data design
- remove WTL shit
- write controls


How to hook the system's color control
- inject my dll into every process and patch IAT (badbear)
- replace comdlg32 (badbear also)


//
////--------------------------------------------------------------------------------

//const int ButtonWidth = 75;
//const int ButtonHeight = 23;
//const int ControlSpacing = 6;


//--------------------------------------------------------------------------------
/*
  * case-sensitive
  * absolute anchors = percentage, pixels, points, etc
  * offset anchors = settings, units, center.  offset anchors adjust the previous absolute anchor.

  // comment
  [0]            <-- absolute anchor
    group.top;    <-- apply to a target
  [100px]         <-- absolute anchor
  [+Button.Width] <-- offset anchor, offsets the last anchor
  [+50px]         <-- offset anchor, offsets the last anchor
    somebutton.top;
  [@Button.Height] <-- apply some preset or template.  these are based off of the UI settings, may adjust the cursor
    somebutton; <-- cannot be a single element
  [100%]
    something.bottom;

  region
  {
    group.top;
    [+Group.TopMargin]
    [@Static.SingleLine.Height]
    oldtext;
    [+Control.Spacing]
    region // this starts an embedded region with cursor bounds starting at the current pos, through 
    {
      oldspot.top;
      [50%]
      oldspot.bottom;
      newspot.top;
      [100%]
      newspot.bottom;
    }
    [+Control.Spacing]
    [@Static.SingleLine.Height]
    newtext;
    [50%]
    [-17px]// hard-coded because i just don't have any support for expressions
    [@Button.Height]
    addtopalette;
    region
    {
      // nothing here!
    }
    [@Button.Height]
    OK;
    [+Control.Spacing]
    [@Button.Height]
    Exit;
    [+Group.BottomMargin]
    [100%]
  }
  
  vertical
  {
    (group.topmargin);
    * OldText.top, SomethingElse.top;
    (static.singleline);
    * OldText.bottom, SomethingElse.bottom;
    (Control.Spacer);
    {
      * oldspot(50%);
      * newspot(50%);
    };
    (Control.Spacer);
    * NewText(static.singleline);
    (50px);
    * AddToPalette(button);
    {
    };
    * Ok(button);
    (control.spacer);
    * Exit(button);
    (group.bottommargin);
  }

  *assignment{children}(distance);

  Warnings:
  - unset target part(s)
  - out of bounds
  - ambiguous anchor

*/
