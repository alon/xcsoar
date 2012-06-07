/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_INFO_BOX_HPP
#define XCSOAR_INFO_BOX_HPP

#include "Util/StaticString.hpp"
#include "InfoBoxes/Content/Base.hpp"
#include "Screen/PaintWindow.hpp"
#include "Screen/Timer.hpp"
#include "PeriodClock.hpp"
#include "Data.hpp"

enum BorderKind_t {
  bkNone,
  bkTop,
  bkRight,
  bkBottom,
  bkLeft
};

#define BORDERTOP    (1<<bkTop)
#define BORDERRIGHT  (1<<bkRight)
#define BORDERBOTTOM (1<<bkBottom)
#define BORDERLEFT   (1<<bkLeft)

struct InfoBoxSettings;
struct InfoBoxLook;
struct UnitsLook;
struct ButtonLook;

class InfoBoxWindow : public PaintWindow
{
  /** timeout of infobox focus [ms] */
  static gcc_constexpr_data unsigned FOCUS_TIMEOUT_MAX = 20 * 1000;

private:
  InfoBoxContent *content;
  ContainerWindow &parent;

  const InfoBoxSettings &settings;
  const InfoBoxLook &look;
  const UnitsLook &units_look;

  /**
   * look to draw "Close" to look like a button over the
   * InfoBox while the access dialog is visible
   */
  const ButtonLook &button_look;

  int border_kind;

  InfoBoxData data;

  int id;

  /**
   * draw the selector event if the InfoBox window is not the system focus
   */
  bool force_draw_selector;

  /** a timer which returns keyboard focus back to the map window after a while */
  WindowTimer focus_timer;

  PixelRect title_rect;
  PixelRect value_rect;
  PixelRect comment_rect;
  PixelRect value_and_comment_rect;

  PeriodClock click_clock;

  /**
   * Paints the InfoBox title to the given canvas
   * @param canvas The canvas to paint on
   */
  void PaintTitle(Canvas &canvas);
  /**
   * Paints the InfoBox value to the given canvas
   * @param canvas The canvas to paint on
   */
  void PaintValue(Canvas &canvas);

  /**
   * Sets font for canvas based on value text
   * Also sets ascent_height, capital_height and value_size
   * based on value text
   * @param canvas The canvas to paint on
   * @param value The text to be displayed
   * @param ascent_height ascent height of text for font to be used
   * @capital_height height of text for font to be used
   * @value_size size of text for font to be used
   */
  void GetValueFont(Canvas &canvas, const TCHAR* value,
                    UPixelScalar &ascent_height,
                    UPixelScalar &capital_height,
                    PixelSize &value_size);

  /**
   * Paints the word "Close" in the InfoBox on the given canvas
   * @param canvas The canvas to paint on
   */
  void PaintClose(Canvas &canvas);

  /**
   * Paints the InfoBox comment on the given canvas
   * @param canvas The canvas to paint on
   */
  void PaintComment(Canvas &canvas);
  /**
   * Paints the InfoBox with borders, title, comment and value
   */
  void Paint(Canvas &canvas);

public:
  void PaintInto(Canvas &dest, PixelScalar xoff, PixelScalar yoff,
                 UPixelScalar width, UPixelScalar height);

  /**
   * Sets the InfoBox ID to the given Value
   * @param id New value of the InfoBox ID
   */
  void SetID(const int id);
  int GetID() { return id; };
  /**
   * Sets the InfoBox title to the given Value
   * @param Value New value of the InfoBox title
   */
  void SetTitle(const TCHAR *title);

  const TCHAR* GetTitle() {
    return data.title;
  };

  /**
   * Constructor of the InfoBoxWindow class
   * @param Parent The parent ContainerWindow (usually MainWindow)
   * @param X x-Coordinate of the InfoBox
   * @param Y y-Coordinate of the InfoBox
   * @param Width Width of the InfoBox
   * @param Height Height of the InfoBox
   */
  InfoBoxWindow(ContainerWindow &parent, PixelScalar x, PixelScalar y,
                UPixelScalar width, UPixelScalar height, int border_flags,
                const InfoBoxSettings &settings, const InfoBoxLook &_look,
                const UnitsLook &units_look, const ButtonLook &button_look,
                WindowStyle style=WindowStyle());

  ~InfoBoxWindow();

  const InfoBoxLook &GetLook() const {
    return look;
  }

  void SetContentProvider(InfoBoxContent *_content);
  void UpdateContent();

protected:
  bool HandleKey(InfoBoxContent::InfoBoxKeyCodes keycode);

public:
  /**
   * This passes a given value to the InfoBoxContent for further processing
   * and updates the InfoBox.
   * @param Value Value to handle
   * @return True on success, Fales otherwise
   */
  bool HandleQuickAccess(const TCHAR *value);

  /**
   * This is a generic handler for the InfoBox. It returns an unsigned value
   * @return a value determined by the content or 0 if no content exists
   */
  virtual gcc_pure unsigned GetQuickAccess();

  const InfoBoxContent::DialogContent *GetDialogContent();

  const PixelRect GetValueRect() const {
    return value_rect;
  }
  const PixelRect GetValueAndCommentRect() const {
    return value_and_comment_rect;
  }

protected:
  virtual void OnDestroy();

  /**
   * This event handler is called when a key is pressed down while the InfoBox
   * is focused
   * @param key_code The code of the key that was pressed
   * @return True if the event has been handled, False otherwise
   */
  virtual bool OnKeyDown(unsigned key_code);
  /**
   * This event handler is called when a mouse button is pressed down over
   * the InfoBox
   * @param x x-Coordinate where the mouse button was pressed
   * @param y y-Coordinate where the mouse button was pressed
   * @return True if the event has been handled, False otherwise
   */
  virtual bool OnMouseDown(PixelScalar x, PixelScalar y);

  virtual bool OnMouseUp(PixelScalar x, PixelScalar y);

  /**
   * This event handler is called when a mouse button is double clicked over
   * the InfoBox
   * @param x x-Coordinate where the mouse button was pressed
   * @param y y-Coordinate where the mouse button was pressed
   * @return True if the event has been handled, False otherwise
   */
  virtual bool OnMouseDouble(PixelScalar x, PixelScalar y);

  virtual void OnResize(UPixelScalar width, UPixelScalar height);

  /**
   * This event handler is called when the InfoBox needs to be repainted
   * @param canvas The canvas to paint on
   */
  virtual void OnPaint(Canvas &canvas);

  virtual bool OnCancelMode();
  virtual void OnSetFocus();
  virtual void OnKillFocus();

  /**
   * This event handler is called when a timer is triggered
   * @param id Id of the timer that triggered the handler
   */
  virtual bool OnTimer(WindowTimer &timer);
};

#endif
