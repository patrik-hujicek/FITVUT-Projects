/* XToolkit/OSF/Motif */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

#include <Xm/ComboBox.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const char *fallbacks[] = {
    "*quit_dialog.dialogTitle: Quit",
    "*quit_dialog.messageString: Quit application?",
    "*quit_dialog.okLabelString: Ok",
    "*quit_dialog.cancelLabelString: Cancel",
    "*quit_dialog.messageAlignment: XmALIGNMENT_CENTER",
    "*.shape_label.labelString: Shape type",
    "*.width_label.labelString: Line width",
    "*.line_label.labelString: Line type",
    "*.line_fg_colour_label.labelString: Line foreground",
    "*.line_bg_colour_label.labelString: Line background",
    "*.fill_fg_colour_label.labelString: Fill foreground",
    "*.fill_bg_colour_label.labelString: Fill background",
    NULL};

// Colours
enum { BLACK, WHITE, RED, BLUE, YELLOW, GREEN };
enum {
  INDEX_LINE_FG_COLOR,
  INDEX_LINE_BG_COLOR,
  INDEX_FILL_FG_COLOR,
  INDEX_FILL_BG_COLOR,
};

char *COLOUR_NAMES[] = {"Black", "White", "Red", "Blue", "Yellow", "Green"};
#define COUNT_COLOURS sizeof(COLOUR_NAMES) / sizeof(char *)
Pixel colors[COUNT_COLOURS];
Pixel line_fg_colour;
Pixel line_bg_colour;
Pixel fill_fg_colour;
Pixel fill_bg_colour;

// Shape types
enum { POINT, LINE, RECTANGLE, FILLED_RECTANGLE, ELLIPSE, FILLED_ELLIPSE };
const size_t INDEX_DEFAULT_SHAPE = 1;

// Widths
const size_t WIDTHS[] = {0, 3, 8};
const size_t INDEX_DEFAULT_WIDTH = 1;

// Line types
enum { SOLID, DOUBLE_DASHED };
const size_t DEFAULT_LINE = 0;

// Button state
enum { RELEASED, PRESSED_UP, PRESSED_DOWN };

// Default colours
const size_t INDEX_DEFAULT_LINE_FG_COLOR = 0;
const size_t INDEX_DEFAULT_FILL_FG_COLOR = 0;
const size_t INDEX_DEFAULT_LINE_BG_COLOR = 1;
const size_t INDEX_DEFAULT_FILL_BG_COLOR = 1;

// Drawing structure
typedef struct drawing {
  Pixel line_fg_colour;
  Pixel line_bg_colour;
  Pixel fill_fg_colour;
  Pixel fill_bg_colour;
  int x1, y1, x2, y2;
  int shape;
  int width;
  int line;
} drawing;

// Draw context
struct draw_context {
  int current_button_state;
  int current_shape;
  int current_width;
  int current_line;
  drawing *drawings;
  drawing current_drawing;
  size_t drawings_allocated_size;
  size_t drawings_count;
} draw_context;

XtAppContext context;
Widget application;
Widget quit_dialog;
Widget draw_area;

void shape_callback(Widget widget, XtPointer client_data, XtPointer call_data) {
  XmToggleButtonCallbackStruct *data =
      (XmToggleButtonCallbackStruct *)call_data;

  if (data->set)
    draw_context.current_shape = (intptr_t)client_data;
}

void width_callback(Widget widget, XtPointer client_data, XtPointer call_data) {
  XmToggleButtonCallbackStruct *data =
      (XmToggleButtonCallbackStruct *)call_data;

  if (data->set)
    draw_context.current_width = WIDTHS[(intptr_t)client_data];
}

void line_callback(Widget widget, XtPointer client_data, XtPointer call_data) {
  XmToggleButtonCallbackStruct *data =
      (XmToggleButtonCallbackStruct *)call_data;

  if (data->set) {
    draw_context.current_line =
        ((intptr_t)client_data == DOUBLE_DASHED) ? LineDoubleDash : LineSolid;
  }
}

void color_callback(Widget widget, XtPointer client_data, XtPointer call_data) {
  XmComboBoxCallbackStruct *data = (XmComboBoxCallbackStruct *)call_data;
  intptr_t type = (intptr_t)client_data;
  Pixel selected_color = colors[data->item_position];

  switch (type) {
  case INDEX_LINE_FG_COLOR:
    line_fg_colour = selected_color;
    break;
  case INDEX_LINE_BG_COLOR:
    line_bg_colour = selected_color;
    break;
  case INDEX_FILL_FG_COLOR:
    fill_fg_colour = selected_color;
    break;
  case INDEX_FILL_BG_COLOR:
    fill_bg_colour = selected_color;
    break;
  }
}

void quit_callback(Widget widget, XtPointer client_data, XtPointer call_data) {
  XtManageChild(quit_dialog);
}

void file_callback(Widget widget, XtPointer client_data, XtPointer call_data) {
  intptr_t cdata = (intptr_t)client_data;
  if (cdata == 0) {
    // Clear drawings
    if (draw_context.drawings) {
      free(draw_context.drawings);
      draw_context.drawings_allocated_size = 0;
      draw_context.drawings_count = 0;
      draw_context.drawings = NULL;
    }

    XClearWindow(XtDisplay(draw_area), XtWindow(draw_area));
  } else if (cdata == 1)
    quit_callback(widget, client_data, call_data);
}

void quit_dialog_callback(Widget widget, XtPointer client_data,
                          XtPointer call_data) {
  if ((intptr_t)client_data == 1) {
    // Clear drawings
    free(draw_context.drawings);
    exit(0);
  }
}

void draw(Widget widget, GC draw_GC, drawing drawing) {
  XGCValues values;
  int width = drawing.width;
  int x1 = drawing.x1;
  int y1 = drawing.y1;
  int x2 = drawing.x2;
  int y2 = drawing.y2;
  values.foreground = drawing.line_fg_colour;
  values.background = drawing.line_bg_colour;
  values.line_width = drawing.width;
  values.line_style = drawing.line;

  XChangeGC(XtDisplay(widget), draw_GC,
            GCLineStyle | GCLineWidth | GCForeground | GCBackground, &values);

  switch (drawing.shape) {
  case POINT:
    if (width > 0)
      XFillArc(XtDisplay(widget), XtWindow(widget), draw_GC,
               (int)(x2 - width / 2.0), (int)(y2 - width / 2.0), width, width,
               0, 360 * 64);
    else
      XDrawPoint(XtDisplay(widget), XtWindow(widget), draw_GC, x2, y2);
    break;
  case LINE:
    XDrawLine(XtDisplay(widget), XtWindow(widget), draw_GC, x1, y1, x2, y2);
    break;
  case RECTANGLE:
    XDrawRectangle(XtDisplay(widget), XtWindow(widget), draw_GC,
                   x1 < x2 ? x1 : x2, y1 < y2 ? y1 : y2, abs(x2 - x1),
                   abs(y2 - y1));
    break;
  case FILLED_RECTANGLE:
    XSetForeground(XtDisplay(widget), draw_GC, drawing.fill_bg_colour);
    XFillRectangle(XtDisplay(widget), XtWindow(widget), draw_GC,
                   x1 < x2 ? x1 : x2, y1 < y2 ? y1 : y2, abs(x2 - x1),
                   abs(y2 - y1));
    XSetForeground(XtDisplay(widget), draw_GC, drawing.line_fg_colour);
    XDrawRectangle(XtDisplay(widget), XtWindow(widget), draw_GC,
                   x1 < x2 ? x1 : x2, y1 < y2 ? y1 : y2, abs(x2 - x1),
                   abs(y2 - y1));
    break;
  case ELLIPSE:
    XDrawArc(XtDisplay(widget), XtWindow(widget), draw_GC, x1 - abs(x2 - x1),
             y1 - abs(y2 - y1), abs(x2 - x1) * 2, abs(y2 - y1) * 2, 0,
             360 * 64);
    break;
  case FILLED_ELLIPSE:
    XSetForeground(XtDisplay(widget), draw_GC, drawing.fill_bg_colour);
    XFillArc(XtDisplay(widget), XtWindow(widget), draw_GC, x1 - abs(x2 - x1),
             y1 - abs(y2 - y1), abs(x2 - x1) * 2, abs(y2 - y1) * 2, 0,
             360 * 64);
    XSetForeground(XtDisplay(widget), draw_GC, drawing.line_fg_colour);
    XDrawArc(XtDisplay(widget), XtWindow(widget), draw_GC, x1 - abs(x2 - x1),
             y1 - abs(y2 - y1), abs(x2 - x1) * 2, abs(y2 - y1) * 2, 0,
             360 * 64);
    break;
  default:
    break;
  }
}

void button_handler(Widget widget, XtPointer client_data, XEvent *event,
                    Boolean *cont) {
  static GC draw_GC = 0;
  Pixel fg, bg;

  if (draw_context.current_button_state != RELEASED) {
    if (!draw_GC) {
      draw_GC = XCreateGC(XtDisplay(widget), XtWindow(widget), 0, NULL);

      XtVaGetValues(widget, XmNforeground, &fg, XmNbackground, &bg, NULL);
      XSetForeground(XtDisplay(widget), draw_GC, fg ^ bg);
      XSetFunction(XtDisplay(widget), draw_GC, GXxor);
      XSetPlaneMask(XtDisplay(widget), draw_GC, ~0);
    }

    if (draw_context.current_button_state == PRESSED_DOWN)
      draw(widget, draw_GC, draw_context.current_drawing);
    else
      draw_context.current_button_state = PRESSED_DOWN;

    draw_context.current_drawing.x2 = event->xmotion.x;
    draw_context.current_drawing.y2 = event->xmotion.y;

    draw(widget, draw_GC, draw_context.current_drawing);
  }
}

void expose_callback(Widget widget, XtPointer client_data,
                     XtPointer call_data) {
  static GC draw_GC = 0;
  if (draw_context.drawings_count) {
    if (!draw_GC)
      draw_GC = XCreateGC(XtDisplay(widget), XtWindow(widget), 0, NULL);

    for (int i = 0; i < draw_context.drawings_count; ++i)
      draw(widget, draw_GC, draw_context.drawings[i]);
  }
}

void draw_callback(Widget widget, XtPointer client_data, XtPointer call_data) {
  XmDrawingAreaCallbackStruct *data = (XmDrawingAreaCallbackStruct *)call_data;
  switch (data->event->type) {
  case ButtonPress:
    if (data->event->xbutton.button == Button1) {
      draw_context.current_drawing.line_fg_colour =
          line_fg_colour ^ colors[INDEX_DEFAULT_LINE_BG_COLOR];
      draw_context.current_drawing.line_bg_colour =
          line_bg_colour ^ colors[INDEX_DEFAULT_LINE_BG_COLOR];
      draw_context.current_drawing.fill_fg_colour =
          fill_fg_colour ^ colors[INDEX_DEFAULT_FILL_FG_COLOR];
      draw_context.current_drawing.fill_bg_colour =
          fill_bg_colour ^ colors[INDEX_DEFAULT_FILL_BG_COLOR];
      draw_context.current_drawing.x1 = data->event->xbutton.x;
      draw_context.current_drawing.y1 = data->event->xbutton.y;

      draw_context.current_drawing.shape = draw_context.current_shape;
      draw_context.current_drawing.width = draw_context.current_width;
      draw_context.current_drawing.line = draw_context.current_line;
      draw_context.current_button_state = PRESSED_UP;
    }
    break;
  case ButtonRelease:
    if (data->event->xbutton.button == Button1) {
      if (draw_context.drawings_count >= draw_context.drawings_allocated_size) {
        draw_context.drawings_allocated_size =
            draw_context.drawings_allocated_size
                ? draw_context.drawings_allocated_size * 2
                : 1;
        draw_context.drawings = (drawing *)XtRealloc(
            (char *)draw_context.drawings,
            draw_context.drawings_allocated_size * sizeof(struct drawing));
      }

      draw_context.drawings[draw_context.drawings_count].line_fg_colour =
          draw_context.current_drawing.line_fg_colour ^
          colors[INDEX_DEFAULT_LINE_BG_COLOR];
      draw_context.drawings[draw_context.drawings_count].line_bg_colour =
          draw_context.current_drawing.line_bg_colour ^
          colors[INDEX_DEFAULT_LINE_BG_COLOR];
      draw_context.drawings[draw_context.drawings_count].fill_fg_colour =
          draw_context.current_drawing.fill_fg_colour ^
          colors[INDEX_DEFAULT_FILL_FG_COLOR];
      draw_context.drawings[draw_context.drawings_count].fill_bg_colour =
          draw_context.current_drawing.fill_bg_colour ^
          colors[INDEX_DEFAULT_FILL_BG_COLOR];
      draw_context.drawings[draw_context.drawings_count].x2 =
          data->event->xbutton.x;
      draw_context.drawings[draw_context.drawings_count].y2 =
          data->event->xbutton.y;
      draw_context.drawings[draw_context.drawings_count].shape =
          draw_context.current_drawing.shape;
      draw_context.drawings[draw_context.drawings_count].width =
          draw_context.current_drawing.width;
      draw_context.drawings[draw_context.drawings_count].line =
          draw_context.current_drawing.line;
      draw_context.drawings[draw_context.drawings_count].x1 =
          draw_context.current_drawing.x1;
      draw_context.drawings[draw_context.drawings_count].y1 =
          draw_context.current_drawing.y1;
      draw_context.current_button_state = RELEASED;
      ++draw_context.drawings_count;

      XClearArea(XtDisplay(widget), XtWindow(widget), 0, 0, 0, 0, True);
    }
    break;
  default:
    break;
  }
}

int init(int argc, char **argv) {
  draw_context.current_shape = INDEX_DEFAULT_SHAPE;
  draw_context.current_line = DEFAULT_LINE;
  draw_context.current_width = WIDTHS[INDEX_DEFAULT_WIDTH];

  application = XtVaAppInitialize(&context, "Draw", NULL, 0, &argc, argv,
                                  (char **)fallbacks, XmNdeleteResponse,
                                  XmDO_NOTHING, NULL);

  XColor xcolor;
  XColor spare;
  Colormap color_map = DefaultColormapOfScreen(XtScreen(application));

  for (int i = 0; i < COUNT_COLOURS; ++i) {
    char *colour = COLOUR_NAMES[i];
    if (XAllocNamedColor(XtDisplay(application), color_map, colour, &xcolor,
                         &spare) == 0) {
      fprintf(stderr, "Failed allocation for color map entry for: %s\n",
              colour);
      return 1;
    }

    colors[i] = xcolor.pixel;
  }

  // Create main window
  Widget main_window = XtVaCreateManagedWidget(
      "main_window", xmMainWindowWidgetClass, application,
      XmNcommandWindowLocation, XmCOMMAND_BELOW_WORKSPACE, XmNwidth, 1200,
      XmNheight, 650, NULL);
  // Initialise colours
  line_fg_colour = colors[INDEX_DEFAULT_LINE_FG_COLOR];
  line_bg_colour = colors[INDEX_DEFAULT_LINE_BG_COLOR];
  fill_fg_colour = colors[INDEX_DEFAULT_FILL_FG_COLOR];
  fill_bg_colour = colors[INDEX_DEFAULT_FILL_BG_COLOR];

  // Create menu items
  XmString file = XmStringCreateSimple("File");
  XmString clear = XmStringCreateSimple("Clear");
  XmString alt_c = XmStringCreateSimple("Alt-C");
  XmString quit = XmStringCreateSimple("Quit");
  XmString alt_q = XmStringCreateSimple("Alt-Q");

  // Create menu bar
  Widget menu_bar = XmVaCreateSimpleMenuBar(
      main_window, "menu_bar", XmVaCASCADEBUTTON, file, XK_F, NULL);

  // Create file menu
  XmVaCreateSimplePulldownMenu(
      menu_bar, "file_menu", 0, file_callback, XmVaPUSHBUTTON, clear, XK_C,
      "Alt<Key>C", alt_c, XmVaPUSHBUTTON, quit, XK_Q, "Alt<Key>Q", alt_q, NULL);

  Widget frame_holder = XtVaCreateManagedWidget(
      "frame_holder", xmFrameWidgetClass, main_window, NULL);

  // Create options
  Widget options = XtVaCreateManagedWidget(
      "options", xmRowColumnWidgetClass, frame_holder, XmNentryAlignment,
      XmALIGNMENT_CENTER, XmNorientation, XmVERTICAL, XmNpacking, XmPACK_TIGHT,
      NULL);

  // Shapes
  XmString point = XmStringCreateSimple("Point");
  XmString line = XmStringCreateSimple("Line");
  XmString rectangle = XmStringCreateSimple("Rectangle");
  XmString filledRectangle = XmStringCreateSimple("Filled rectangle");
  XmString ellipse = XmStringCreateSimple("Ellipse");
  XmString filledEllipse = XmStringCreateSimple("Filled ellipse");
  XtVaCreateManagedWidget("shape_label", xmLabelWidgetClass, options, NULL);
  Widget shape_radio = XmVaCreateSimpleRadioBox(
      options, "shape_radio", INDEX_DEFAULT_SHAPE, shape_callback,
      XmVaRADIOBUTTON, point, NULL, NULL, NULL, XmVaRADIOBUTTON, line, NULL,
      NULL, NULL, XmVaRADIOBUTTON, rectangle, NULL, NULL, NULL, XmVaRADIOBUTTON,
      filledRectangle, NULL, NULL, NULL, XmVaRADIOBUTTON, ellipse, NULL, NULL,
      NULL, XmVaRADIOBUTTON, filledEllipse, NULL, NULL, NULL, NULL);

  // Widths
  XmString width_0px = XmStringCreateSimple("0 px");
  XmString width_3px = XmStringCreateSimple("3 px");
  XmString width_8px = XmStringCreateSimple("8 px");
  XtVaCreateManagedWidget("width_label", xmLabelWidgetClass, options, NULL);
  Widget width_radio = XmVaCreateSimpleRadioBox(
      options, "width_radio", INDEX_DEFAULT_WIDTH, width_callback,
      XmVaRADIOBUTTON, width_0px, NULL, NULL, NULL, XmVaRADIOBUTTON, width_3px,
      NULL, NULL, NULL, XmVaRADIOBUTTON, width_8px, NULL, NULL, NULL, NULL);

  XmString solid = XmStringCreateSimple("Solid");
  XmString double_dashed = XmStringCreateSimple("Double dashed");
  XtVaCreateManagedWidget("line_label", xmLabelWidgetClass, options, NULL);
  Widget line_radio = XmVaCreateSimpleRadioBox(
      options, "line_radio", 0, line_callback, XmVaRADIOBUTTON, solid, NULL,
      NULL, NULL, XmVaRADIOBUTTON, double_dashed, NULL, NULL, NULL, NULL);

  // Colors
  XmString black = XmStringCreateSimple("Black");
  XmString white = XmStringCreateSimple("White");
  XmString red = XmStringCreateSimple("Red");
  XmString blue = XmStringCreateSimple("Blue");
  XmString yellow = XmStringCreateSimple("Yellow");
  XmString green = XmStringCreateSimple("Green");
  XmString colorStrings[] = {black, white, red, blue, yellow, green};

  // Line background
  XtVaCreateManagedWidget("line_bg_colour_label", xmLabelWidgetClass, options,
                          NULL);
  Widget line_bg_colour_combo = XtVaCreateManagedWidget(
      "line_bg_colour_combo", xmComboBoxWidgetClass, options, XmNcomboBoxType,
      XmDROP_DOWN_LIST, XmNitemCount, COUNT_COLOURS, XmNitems, colorStrings,
      XmNselectedPosition, INDEX_DEFAULT_LINE_BG_COLOR, NULL);

  // Fill background
  XtVaCreateManagedWidget("fill_bg_colour_label", xmLabelWidgetClass, options,
                          NULL);
  Widget fill_bg_colour_combo = XtVaCreateManagedWidget(
      "fill_bg_colour_combo", xmComboBoxWidgetClass, options, XmNcomboBoxType,
      XmDROP_DOWN_LIST, XmNitemCount, COUNT_COLOURS, XmNitems, colorStrings,
      XmNselectedPosition, INDEX_DEFAULT_FILL_BG_COLOR, NULL);

  // Line foreground
  XtVaCreateManagedWidget("line_fg_colour_label", xmLabelWidgetClass, options,
                          NULL);
  Widget line_fg_colour_combo = XtVaCreateManagedWidget(
      "line_fg_colour_combo", xmComboBoxWidgetClass, options, XmNcomboBoxType,
      XmDROP_DOWN_LIST, XmNitemCount, COUNT_COLOURS, XmNitems, colorStrings,
      XmNselectedPosition, INDEX_DEFAULT_LINE_FG_COLOR, NULL);

  // Fill foreground
  XtVaCreateManagedWidget("fill_fg_colour_label", xmLabelWidgetClass, options,
                          NULL);
  Widget fill_fg_colour_combo = XtVaCreateManagedWidget(
      "fill_fg_colour_combo", xmComboBoxWidgetClass, options, XmNcomboBoxType,
      XmDROP_DOWN_LIST, XmNitemCount, COUNT_COLOURS, XmNitems, colorStrings,
      XmNselectedPosition, INDEX_DEFAULT_FILL_FG_COLOR, NULL);

  // Add color callbacks
  XtAddCallback(line_fg_colour_combo, XmNselectionCallback, color_callback,
                (XtPointer)INDEX_LINE_FG_COLOR);
  XtAddCallback(line_bg_colour_combo, XmNselectionCallback, color_callback,
                (XtPointer)INDEX_LINE_BG_COLOR);
  XtAddCallback(fill_fg_colour_combo, XmNselectionCallback, color_callback,
                (XtPointer)INDEX_FILL_FG_COLOR);
  XtAddCallback(fill_bg_colour_combo, XmNselectionCallback, color_callback,
                (XtPointer)INDEX_FILL_BG_COLOR);

  // Create draw area
  draw_area =
      XtVaCreateManagedWidget("draw_area", xmDrawingAreaWidgetClass,
                              frame_holder, XmNbackground, colors[WHITE], NULL);

  XtAddEventHandler(draw_area, ButtonMotionMask, False, button_handler, NULL);
  XtAddCallback(draw_area, XmNexposeCallback, expose_callback, draw_area);
  XtAddCallback(draw_area, XmNinputCallback, draw_callback, draw_area);

  quit_dialog = XmCreateQuestionDialog(application, "quit_dialog", NULL, 0);
  XtVaSetValues(quit_dialog, XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
                NULL);
  XtUnmanageChild(XmMessageBoxGetChild(quit_dialog, XmDIALOG_HELP_BUTTON));
  XtAddCallback(quit_dialog, XmNcancelCallback, quit_dialog_callback,
                (XtPointer)0);
  XtAddCallback(quit_dialog, XmNokCallback, quit_dialog_callback, (XtPointer)1);

  XtManageChild(main_window);
  XtManageChild(menu_bar);
  XtManageChild(shape_radio);
  XtManageChild(width_radio);
  XtManageChild(line_radio);
  XtManageChild(draw_area);

  // Free memory
  XmStringFree(file);
  XmStringFree(clear);
  XmStringFree(alt_c);
  XmStringFree(quit);
  XmStringFree(alt_q);

  XmStringFree(black);
  XmStringFree(white);
  XmStringFree(red);
  XmStringFree(blue);
  XmStringFree(yellow);
  XmStringFree(green);

  XmStringFree(solid);
  XmStringFree(double_dashed);

  XmStringFree(width_0px);
  XmStringFree(width_3px);
  XmStringFree(width_8px);

  XmStringFree(point);
  XmStringFree(line);
  XmStringFree(rectangle);
  XmStringFree(filledRectangle);
  XmStringFree(ellipse);
  XmStringFree(filledEllipse);
  return 0;
}

int main(int argc, char **argv) {
  int init_ret = init(argc, argv);
  if (init_ret) // Error occurred
    return init_ret;

  Atom wmDelete =
      XInternAtom(XtDisplay(application), "WM_DELETE_WINDOW", False);
  XmAddWMProtocolCallback(application, wmDelete, quit_callback, NULL);
  XmActivateWMProtocol(application, wmDelete);

  XtSetLanguageProc(NULL, (XtLanguageProc)NULL, NULL);
  XtRealizeWidget(application);
  XtAppMainLoop(context);

  // Free any created drawings
  free(draw_context.drawings);
  return 0;
}
