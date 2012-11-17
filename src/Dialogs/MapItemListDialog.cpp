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

#include "Dialogs/MapItemListDialog.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Screen/Layout.hpp"
#include "Dialogs/Airspace.hpp"
#include "Dialogs/Task.hpp"
#include "Dialogs/Waypoint.hpp"
#include "Dialogs/Traffic.hpp"
#include "Look/DialogLook.hpp"
#include "Look/Look.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "MapSettings.hpp"
#include "MapWindow/MapItem.hpp"
#include "MapWindow/MapItemList.hpp"
#include "Renderer/MapItemListRenderer.hpp"
#include "Form/ListWidget.hpp"
#include "Form/Button.hpp"
#include "Weather/Features.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Interface.hpp"

#ifdef HAVE_NOAA
#include "Dialogs/Weather.hpp"
#endif

static bool
HasDetails(const MapItem &item)
{
  switch (item.type) {
  case MapItem::LOCATION:
  case MapItem::ARRIVAL_ALTITUDE:
  case MapItem::SELF:
  case MapItem::MARKER:
  case MapItem::THERMAL:
    return false;

  case MapItem::AIRSPACE:
  case MapItem::WAYPOINT:
  case MapItem::TASK_OZ:
  case MapItem::TRAFFIC:
#ifdef HAVE_NOAA
  case MapItem::WEATHER:
#endif
    return true;
  }

  return false;
}

/**
 * returns true if item type is displayed in header above list
 */
static bool
IsInHeader(const MapItem &item)
{
  switch (item.type) {
  case MapItem::LOCATION:
  case MapItem::ARRIVAL_ALTITUDE:
  case MapItem::SELF:
    return true;

  case MapItem::MARKER:
  case MapItem::THERMAL:
  case MapItem::AIRSPACE:
  case MapItem::WAYPOINT:
  case MapItem::TASK_OZ:
  case MapItem::TRAFFIC:
#ifdef HAVE_NOAA
  case MapItem::WEATHER:
#endif
    return false;
  }

  return false;
}

/**
 * A class based on the WidgetDialog that draws map list items
 * about the plane in the header, and map list items that exist
 * on the map as separate entities as items in a list.
 */
class MapItemListDialog : public WidgetDialog
{
protected:
  const MapItemList &header_list;
  const Look &look;
  const MapSettings &settings;

public:
  MapItemListDialog(const TCHAR *caption, Widget *widget,
                    UPixelScalar header_height,
                    const MapItemList &_header_list,
                    const MapSettings &_settings)
    :WidgetDialog(caption, widget, header_height),
     header_list(_header_list),
     look(UIGlobals::GetLook()),
     settings(_settings)
     {}

  /**
   * paints the header of the Dialog
   */
  virtual void OnPaint(Canvas &canvas)
  {
    WidgetDialog::OnPaint(canvas);

    const DialogLook &dialog_look = look.dialog;
    const TrafficLook &traffic_look = look.traffic;
    const FinalGlideBarLook &final_glide_look = look.final_glide_bar;

    /* header_caption is painted at the bottom of the header */
    UPixelScalar header_caption_height = dialog_look.caption.font->GetHeight();
    UPixelScalar item_height = dialog_look.list.font->GetHeight()
      + Layout::Scale(6) + dialog_look.small_font->GetHeight();
    assert(item_height > 0);

    const PixelRect rc_header = GetHeaderRect();
    PixelRect rc_footer_caption = rc_header;
    rc_footer_caption.top = rc_footer_caption.bottom - header_caption_height;

    Brush brush(COLOR_XCSOAR_DARK);
    canvas.Select(brush);
    canvas.SetTextColor(COLOR_WHITE);
    canvas.Select(*dialog_look.caption.font);
    canvas.Rectangle(rc_footer_caption.left, rc_footer_caption.top,
                     rc_footer_caption.right, rc_footer_caption.bottom);
    canvas.text(rc_footer_caption.left + Layout::FastScale(2),
                rc_footer_caption.top, _("Nearby items:"));

    PixelRect header_rect_inner = rc_header;
    header_rect_inner.bottom -= header_caption_height;

    canvas.SelectWhiteBrush();
    canvas.Rectangle(header_rect_inner.left, header_rect_inner.top,
                     header_rect_inner.right, header_rect_inner.bottom);

    canvas.SetTextColor(COLOR_BLACK);
    for (unsigned i = 0; i < header_list.size(); i++) {
      MapItem& item = *header_list[i];
      PixelRect rc = header_rect_inner;
      rc.left += Layout::Scale(1);
      rc.top += i * item_height;
      rc.bottom = rc.top + item_height;
      MapItemListRenderer::Draw(canvas, rc, item,
                                dialog_look, look.map, traffic_look,
                                final_glide_look, settings,
                                &XCSoarInterface::Basic().flarm.traffic);
    }

  }
};

class MapItemListWidget : public ListWidget, private ActionListener {
  enum Buttons {
    SETTINGS,
    GOTO,
  };

  const MapItemList &list;

  const DialogLook &dialog_look;
  const MapLook &look;
  const TrafficLook &traffic_look;
  const FinalGlideBarLook &final_glide_look;
  const MapSettings &settings;

  WndButton *settings_button, *details_button, *cancel_button, *goto_button;

public:
  void CreateButtons(WidgetDialog &dialog);

public:
  MapItemListWidget(const MapItemList &_list,
                    const DialogLook &_dialog_look, const MapLook &_look,
                    const TrafficLook &_traffic_look,
                    const FinalGlideBarLook &_final_glide_look,
                    const MapSettings &_settings)
    :list(_list),
     dialog_look(_dialog_look), look(_look),
     traffic_look(_traffic_look), final_glide_look(_final_glide_look),
     settings(_settings) {}

  unsigned GetCursorIndex() const {
    return GetList().GetCursorIndex();
  }

protected:
  void UpdateButtons() {
    const unsigned current = GetCursorIndex();
    if (list.size() == 0) {
      details_button->SetEnabled(false);
      goto_button->SetEnabled(false);
      return;
    }
    details_button->SetEnabled(HasDetails(*list[current]));
    goto_button->SetEnabled(CanGotoItem(current));
  }

public:
  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare() {
    DeleteWindow();
  }

  /* virtual methods from class List::Handler */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx);

  virtual void OnCursorMoved(unsigned index) {
    UpdateButtons();
  }

  virtual bool CanActivateItem(unsigned index) const {
    return HasDetails(*list[index]);
  }

  bool CanGotoItem(unsigned index) const {
    assert(index < list.size());
    return protected_task_manager != NULL &&
      list[index]->type == MapItem::WAYPOINT;
  }

  virtual void OnActivateItem(unsigned index);

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id);
};

void
MapItemListWidget::CreateButtons(WidgetDialog &dialog)
{
  settings_button = dialog.AddButton(_("Settings"), this, SETTINGS);
  goto_button = dialog.AddButton(_("Goto"), this, GOTO);
  details_button = dialog.AddButton(_("Details"), mrOK);
  cancel_button = dialog.AddButton(_("Close"), mrCancel);
}

void
MapItemListWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  UPixelScalar item_height = dialog_look.list.font->GetHeight()
    + Layout::Scale(6) + dialog_look.small_font->GetHeight();
  assert(item_height > 0);

  CreateList(parent, dialog_look, rc, item_height);

  GetList().SetLength(list.size());
  UpdateButtons();
}

void
MapItemListWidget::OnPaintItem(Canvas &canvas, const PixelRect rc,
                               unsigned idx)
{
  const MapItem &item = *list[idx];
  MapItemListRenderer::Draw(canvas, rc, item,
                            dialog_look, look, traffic_look,
                            final_glide_look, settings,
                            &XCSoarInterface::Basic().flarm.traffic);

  if ((settings.item_list.add_arrival_altitude &&
       item.type == MapItem::Type::ARRIVAL_ALTITUDE) ||
      (!settings.item_list.add_arrival_altitude &&
       item.type == MapItem::Type::LOCATION)) {
    canvas.SelectBlackPen();
    canvas.DrawLine(rc.left, rc.bottom - 1, rc.right, rc.bottom - 1);
  }
}

void
MapItemListWidget::OnActivateItem(unsigned index)
{
  details_button->OnClicked();
}

void
MapItemListWidget::OnAction(int id)
{
  switch (id) {
  case SETTINGS:
    ShowMapItemListSettingsDialog();
    break;
  case GOTO:
    if (protected_task_manager == NULL)
      break;

    unsigned index = GetCursorIndex();
    auto const &item = *list[index];

    assert(item.type == MapItem::WAYPOINT);

    auto const &waypoint = ((const WaypointMapItem &)item).waypoint;
    protected_task_manager->DoGoto(waypoint);
    cancel_button->OnClicked();

    break;
  }
}

/**
 * Populates header_list with items the belong in the header
 * Populates list_list with items that belong in the lists
 *
 * @param header_list. the list of all the Map Items
 * @param header_list. an empty list
 * @param list_list. an empty list
 */
static void
SplitMapItemList(const MapItemList &full_list,
                 MapItemList& header_list,
                 MapItemList& list_list)
{
  for (unsigned i = 0; i < full_list.size(); i++) {
    MapItem &item = *full_list[i];
    if (IsInHeader(item))
      header_list.append(&item);
    else
      list_list.append(&item);
  }
}

static int
ShowMapItemListDialog(SingleWindow &parent,
                      const MapItemList &list,
                      const DialogLook &dialog_look, const MapLook &look,
                      const TrafficLook &traffic_look,
                      const FinalGlideBarLook &final_glide_look,
                      const MapSettings &settings)
{
  MapItemList list_list;
  MapItemList header_list;
  list_list.clear();
  header_list.clear();
  SplitMapItemList(list, header_list, list_list);

  UPixelScalar item_height = dialog_look.list.font->GetHeight()
    + Layout::Scale(6) + dialog_look.small_font->GetHeight();
  assert(item_height > 0);
  UPixelScalar header_caption_height = dialog_look.caption.font->GetHeight();

  MapItemListWidget widget(list_list, dialog_look, look,
                           traffic_look, final_glide_look,
                           settings);
  MapItemListDialog dialog(_("Point:"), &widget,
                           item_height * header_list.size()
                           + header_caption_height,
                           header_list, settings);

  widget.CreateButtons(dialog);

  int result = dialog.ShowModal() == mrOK
    ? (int)widget.GetCursorIndex() + header_list.size()
    : -1;
  dialog.StealWidget();
  list_list.clear();
  header_list.clear();
  return result;
}

static void
ShowMapItemDialog(const MapItem &item, SingleWindow &parent,
                  ProtectedAirspaceWarningManager *airspace_warnings)
{
  switch (item.type) {
  case MapItem::LOCATION:
  case MapItem::ARRIVAL_ALTITUDE:
  case MapItem::SELF:
  case MapItem::MARKER:
  case MapItem::THERMAL:
    break;

  case MapItem::AIRSPACE:
    dlgAirspaceDetails(*((const AirspaceMapItem &)item).airspace,
                       airspace_warnings);
    break;
  case MapItem::WAYPOINT:
    dlgWaypointDetailsShowModal(parent,
                                ((const WaypointMapItem &)item).waypoint);
    break;
  case MapItem::TASK_OZ:
    dlgTargetShowModal(((const TaskOZMapItem &)item).index);
    break;
  case MapItem::TRAFFIC:
    dlgFlarmTrafficDetailsShowModal(((const TrafficMapItem &)item).id);
    break;

#ifdef HAVE_NOAA
  case MapItem::WEATHER:
    dlgNOAADetailsShowModal(parent,
                            ((const WeatherStationMapItem &)item).station);
    break;
#endif
  }
}

void
ShowMapItemListDialog(SingleWindow &parent,
                      const MapItemList &list,
                      const DialogLook &dialog_look,
                      const MapLook &look,
                      const TrafficLook &traffic_look,
                      const FinalGlideBarLook &final_glide_look,
                      const MapSettings &settings,
                      ProtectedAirspaceWarningManager *airspace_warnings)
{
  switch (list.size()) {
  case 0:
    /* no map items in the list */
    return;

  case 1:
    /* only one map item, show it */
    ShowMapItemDialog(*list[0], parent, airspace_warnings);
    break;

  default:
    /* more than one map item: show a list */

    int i = ShowMapItemListDialog(parent, list, dialog_look, look,
                                  traffic_look, final_glide_look, settings);
    assert(i >= -1 && i < (int)list.size());
    if (i >= 0)
      ShowMapItemDialog(*list[i], parent, airspace_warnings);
  }
}
