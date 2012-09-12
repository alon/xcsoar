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

#include "ChartRenderer.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Language/Language.hpp"
#include "Math/FastMath.h"
#include "Math/FastRotation.hpp"
#include "Asset.hpp"
#include "Math/LeastSquares.hpp"
#include "Util/StaticString.hpp"

#include <assert.h>
#include <stdio.h>
#include <windef.h> /* for MAX_PATH */

void
ChartRenderer::ResetScale()
{
  unscaled_y = true;
  unscaled_x = true;
  xscale = fixed_zero;
  yscale = fixed_zero;
  x_min = fixed_zero;
  x_max = fixed_zero;
  y_min = fixed_zero;
  y_max = fixed_zero;
}

ChartRenderer::ChartRenderer(const ChartLook &_look, Canvas &the_canvas,
                             const PixelRect the_rc)
  :look(_look), canvas(the_canvas), rc(the_rc),
   padding_left(24), padding_bottom(19)
{
  ResetScale();
}

void
ChartRenderer::ScaleYFromData(const LeastSquares &lsdata)
{
  if (lsdata.IsEmpty())
    return;

  if (unscaled_y) {
    y_min = lsdata.y_min;
    y_max = lsdata.y_max;
    unscaled_y = false;
  } else {
    y_min = min(y_min, lsdata.y_min);
    y_max = max(y_max, lsdata.y_max);
  }

  if (lsdata.sum_n > 1) {
    fixed y0, y1;
    y0 = lsdata.x_min * lsdata.m + lsdata.b;
    y1 = lsdata.x_max * lsdata.m + lsdata.b;
    y_min = min(y_min, min(y0, y1));
    y_max = max(y_max, max(y0, y1));
  }

  if (fabs(y_max - y_min) > fixed(50)) {
    yscale = (y_max - y_min);
    if (positive(yscale))
      yscale = fixed(rc.bottom - rc.top - padding_bottom) / yscale;
  } else {
    yscale = fixed(2000);
  }
}

void
ChartRenderer::ScaleXFromData(const LeastSquares &lsdata)
{
  if (lsdata.IsEmpty())
    return;

  if (unscaled_x) {
    x_min = lsdata.x_min;
    x_max = lsdata.x_max;
    unscaled_x = false;
  } else {
    x_min = min(x_min, lsdata.x_min);
    x_max = max(x_max, lsdata.x_max);
  }

  xscale = (x_max - x_min);
  if (positive(xscale))
    xscale = fixed(rc.right - rc.left - padding_left) / xscale;
}

void
ChartRenderer::ScaleYFromValue(const fixed value)
{
  if (unscaled_y) {
    y_min = value;
    y_max = value;
    unscaled_y = false;
  } else {
    y_min = min(value, y_min);
    y_max = max(value, y_max);
  }

  yscale = (y_max - y_min);
  if (positive(yscale))
    yscale = fixed(rc.bottom - rc.top - padding_bottom) / yscale;
}

void
ChartRenderer::ScaleXFromValue(const fixed value)
{
  if (unscaled_x) {
    x_min = value;
    x_max = value;
    unscaled_x = false;
  } else {
    x_min = min(value, x_min);
    x_max = max(value, x_max);
  }

  xscale = (x_max - x_min);
  if (positive(xscale))
    xscale = fixed(rc.right - rc.left - padding_left) / xscale;
}

void
ChartRenderer::StyleLine(const RasterPoint l1, const RasterPoint l2,
                         ChartLook::Style Style)
{
  StyleLine(l1, l2, look.GetPen(Style));
}

void
ChartRenderer::StyleLine(const RasterPoint l1, const RasterPoint l2,
                         const Pen &pen)
{
  assert(pen.IsDefined());
  canvas.Select(pen);
  canvas.DrawLine(l1, l2);
}

void
ChartRenderer::DrawLabel(const TCHAR *text, const fixed xv, const fixed yv)
{
  canvas.Select(*look.label_font);
  canvas.SetBackgroundTransparent();

  PixelSize tsize = canvas.CalcTextSize(text);
  RasterPoint pt = ToScreen(xv, yv);
  canvas.text(pt.x - tsize.cx / 2, pt.y - tsize.cy / 2, text);
}

void
ChartRenderer::DrawNoData()
{
  canvas.Select(*look.label_font);
  canvas.SetBackgroundTransparent();

  const TCHAR *text = _("No data");
  PixelSize tsize = canvas.CalcTextSize(text);

  PixelScalar x = (rc.left + rc.right - tsize.cx) / 2;
  PixelScalar y = (rc.top + rc.bottom - tsize.cy) / 2;

  canvas.text(x, y, text);
}

void
ChartRenderer::DrawXLabel(const TCHAR *text)
{
  canvas.Select(*look.axis_label_font);
  canvas.SetBackgroundTransparent();

  PixelSize tsize = canvas.CalcTextSize(text);
  PixelScalar x = rc.right - tsize.cx - Layout::Scale(3);
  PixelScalar y = rc.bottom - tsize.cy;

  canvas.text(x, y, text);
}

void
ChartRenderer::DrawXLabel(const TCHAR *text, const TCHAR *unit)
{
  assert(text != NULL);
  assert(unit != NULL);

  StaticString<64> buffer;
  buffer.UnsafeFormat(_T("%s [%s]"), text, unit);
  DrawXLabel(buffer);
}

void
ChartRenderer::DrawYLabel(const TCHAR *text)
{
  canvas.Select(*look.axis_label_font);
  canvas.SetBackgroundTransparent();

  PixelSize tsize = canvas.CalcTextSize(text);
  PixelScalar x = max(PixelScalar(2), PixelScalar(rc.left - tsize.cx));
  PixelScalar y = rc.top;

  canvas.text(x, y, text);
}

void
ChartRenderer::DrawYLabel(const TCHAR *text, const TCHAR *unit)
{
  assert(text != NULL);
  assert(unit != NULL);

  StaticString<64> buffer;
  buffer.UnsafeFormat(_T("%s [%s]"), text, unit);
  DrawYLabel(buffer);
}

void
ChartRenderer::DrawTrend(const LeastSquares &lsdata, ChartLook::Style Style)
{
  if (lsdata.sum_n < 2)
    return;

  if (unscaled_x || unscaled_y)
    return;

  fixed xmin, xmax, ymin, ymax;
  xmin = lsdata.x_min;
  xmax = lsdata.x_max;
  ymin = lsdata.x_min * lsdata.m + lsdata.b;
  ymax = lsdata.x_max * lsdata.m + lsdata.b;

  xmin = (xmin - x_min) * xscale + fixed(rc.left + padding_left);
  xmax = (xmax - x_min) * xscale + fixed(rc.left + padding_left);
  ymin = (y_max - ymin) * yscale + fixed(rc.top);
  ymax = (y_max - ymax) * yscale + fixed(rc.top);

  RasterPoint line[2];
  line[0].x = (int)xmin;
  line[0].y = (int)ymin;
  line[1].x = (int)xmax;
  line[1].y = (int)ymax;

  StyleLine(line[0], line[1], Style);
}

void
ChartRenderer::DrawTrendN(const LeastSquares &lsdata, ChartLook::Style Style)
{
  if (lsdata.sum_n < 2)
    return;

  if (unscaled_x || unscaled_y)
    return;

  fixed xmin, xmax, ymin, ymax;
  xmin = fixed_half;
  xmax = fixed(lsdata.sum_n) + fixed_half;
  ymin = lsdata.x_min * lsdata.m + lsdata.b;
  ymax = lsdata.x_max * lsdata.m + lsdata.b;

  xmin = (xmin) * xscale + fixed(rc.left + padding_left);
  xmax = (xmax) * xscale + fixed(rc.left + padding_left);
  ymin = (y_max - ymin * yscale) + fixed(rc.top);
  ymax = (y_max - ymax * yscale) + fixed(rc.top);

  RasterPoint line[2];
  line[0].x = (int)xmin;
  line[0].y = (int)ymin;
  line[1].x = (int)xmax;
  line[1].y = (int)ymax;

  StyleLine(line[0], line[1], Style);
}

void
ChartRenderer::DrawLine(const fixed xmin, const fixed ymin,
                        const fixed xmax, const fixed ymax, const Pen &pen)
{
  if (unscaled_x || unscaled_y)
    return;

  StyleLine(ToScreen(xmin, ymin), ToScreen(xmax, ymax), pen);
}

void 
ChartRenderer::DrawFilledLine(const fixed xmin, const fixed ymin,
                              const fixed xmax, const fixed ymax,
                              const Brush &brush)
{
  RasterPoint line[4];

  line[0] = ToScreen(xmin, ymin);
  line[1] = ToScreen(xmax, ymax);

  line[2].x = line[1].x;
  line[2].y = (int)((y_max) * yscale) + rc.top;
  line[3].x = line[0].x;
  line[3].y = (int)((y_max) * yscale) + rc.top;

  canvas.Select(brush);
  canvas.SelectNullPen();
  canvas.DrawTriangleFan(line, 4);
}

void
ChartRenderer::DrawLine(const fixed xmin, const fixed ymin,
                        const fixed xmax, const fixed ymax,
                        ChartLook::Style Style)
{
  DrawLine(xmin, ymin, xmax, ymax, look.GetPen(Style));
}

void
ChartRenderer::DrawBarChart(const LeastSquares &lsdata)
{
  if (unscaled_x || unscaled_y)
    return;

  Brush green_brush(look.color_theme->green);
  canvas.Select(green_brush);
  canvas.SelectNullPen();

  for (unsigned i = 0, n = lsdata.slots.size(); i != n; i++) {
    PixelScalar xmin((fixed(i) + fixed(1.2)) * xscale
                     + fixed(rc.left + padding_left));
    PixelScalar ymin((y_max - y_min) * yscale + fixed(rc.top));
    PixelScalar xmax((fixed(i) + fixed(1.8)) * xscale
                     + fixed(rc.left + padding_left));
    PixelScalar ymax((y_max - lsdata.slots[i].y) * yscale + fixed(rc.top));
    canvas.Rectangle(xmin, ymin, xmax, ymax);
  }
}

void
ChartRenderer::DrawFilledLineGraph(const LeastSquares &lsdata)
{
  assert(lsdata.slots.size() >= 2);

  const unsigned n = lsdata.slots.size() + 2;
  RasterPoint *points = point_buffer.get(n);

  RasterPoint *p = points;
  for (auto i = lsdata.slots.begin(), end = lsdata.slots.end();
       i != end; ++i)
    *p++ = ToScreen(i->x, i->y);
  *p++ = RasterPoint{ p[-1].x, PixelScalar(rc.bottom - padding_bottom) };
  *p++ = RasterPoint{ points[0].x, PixelScalar(rc.bottom - padding_bottom) };

  assert(p == points + n);

  canvas.DrawPolygon(points, n);
}

void
ChartRenderer::DrawLineGraph(const LeastSquares &lsdata, const Pen &pen)
{
  assert(lsdata.slots.size() >= 2);

  const unsigned n = lsdata.slots.size();
  RasterPoint *points = point_buffer.get(n);

  RasterPoint *p = points;
  for (auto i = lsdata.slots.begin(), end = lsdata.slots.end();
       i != end; ++i)
    *p++ = ToScreen(i->x, i->y);
  assert(p == points + n);

  canvas.Select(pen);
  canvas.DrawPolyline(points, n);
}

void
ChartRenderer::DrawLineGraph(const LeastSquares &lsdata,
                             ChartLook::Style Style)
{
  DrawLineGraph(lsdata, look.GetPen(Style));
}

void
ChartRenderer::FormatTicText(TCHAR *text, const fixed val, const fixed step)
{
  if (step < fixed_one) {
    _stprintf(text, _T("%.1f"), (double)val);
  } else {
    _stprintf(text, _T("%.0f"), (double)val);
  }
}

void
ChartRenderer::DrawXGrid(const fixed tic_step, const fixed zero,
                         ChartLook::Style Style,
                         const fixed unit_step, bool draw_units)
{
  DrawXGrid(tic_step, zero, look.GetPen(Style), unit_step, draw_units);
}

void
ChartRenderer::DrawXGrid(fixed tic_step, const fixed zero, const Pen &pen,
                         fixed unit_step, bool draw_units)
{
  if (!positive(tic_step))
    return;

  canvas.Select(pen);
  canvas.Select(*look.axis_value_font);
  canvas.SetBackgroundTransparent();

  RasterPoint line[2];

  /** the minimum next position of the text, to avoid overlapping */
  PixelScalar next_text = rc.left;

  /* increase tic step so graph not too crowded */
  while ((x_max-x_min)/tic_step > fixed_ten) {
    tic_step *= fixed_two;
    unit_step *= fixed_two;
  }
  //  bool do_units = ((x_max-zero)/tic_step)<10;

  line[0].y = rc.top;
  line[1].y = rc.bottom - padding_bottom;

  for (fixed xval = zero; xval <= x_max; xval += tic_step) {
    const PixelScalar xmin = ScreenX(xval);
    line[0].x = line[1].x = xmin;

    // STYLE_THINDASHPAPER
    if ((xval < x_max) && (xmin >= rc.left + padding_left) && (xmin <= rc.right)) {
      canvas.DrawLine(line[0], line[1]);

      if (draw_units && xmin >= next_text) {
        TCHAR unit_text[MAX_PATH];
        FormatTicText(unit_text, xval * unit_step / tic_step, unit_step);

        canvas.text(xmin, rc.bottom - Layout::Scale(17), unit_text);

        next_text = xmin + canvas.CalcTextSize(unit_text).cx + Layout::FastScale(2);
      }
    }
  }

  for (fixed xval = zero - tic_step; xval >= x_min; xval -= tic_step) {
    const PixelScalar xmin = ScreenX(xval);
    line[0].x = line[1].x = xmin;

    // STYLE_THINDASHPAPER

    if ((xval > x_min) && (xmin >= rc.left + padding_left) && (xmin <= rc.right)) {
      canvas.DrawLine(line[0], line[1]);

      if (draw_units) {
        TCHAR unit_text[MAX_PATH];
        FormatTicText(unit_text, xval * unit_step / tic_step, unit_step);

        canvas.text(xmin, rc.bottom - Layout::Scale(17), unit_text);
      }
    }
  }
}

void
ChartRenderer::DrawYGrid(const fixed tic_step, const fixed zero,
                         ChartLook::Style Style,
                         const fixed unit_step, bool draw_units)
{
  DrawYGrid(tic_step, zero, look.GetPen(Style), unit_step, draw_units);
}

void
ChartRenderer::DrawYGrid(fixed tic_step, const fixed zero, const Pen &pen,
                         fixed unit_step, bool draw_units)
{
  if (!positive(tic_step))
    return;

  canvas.Select(pen);
  canvas.Select(*look.axis_value_font);
  canvas.SetBackgroundTransparent();

  RasterPoint line[2];

  /* increase tic step so graph not too crowded */
  while ((y_max-y_min)/tic_step > fixed_ten) {
    tic_step *= fixed_two;
    unit_step *= fixed_two;
  }

  line[0].x = rc.left + padding_left;
  line[1].x = rc.right;

  for (fixed yval = zero; yval <= y_max; yval += tic_step) {
    const PixelScalar ymin = ScreenY(yval);
    line[0].y = line[1].y = ymin;

    // STYLE_THINDASHPAPER
    if ((yval < y_max) && (ymin >= rc.top) && (ymin <= rc.bottom - padding_bottom)) {
      canvas.DrawLine(line[0], line[1]);

      if (draw_units) {
        TCHAR unit_text[MAX_PATH];
        FormatTicText(unit_text, yval * unit_step / tic_step, unit_step);

        canvas.text(rc.left + Layout::Scale(8), ymin, unit_text);
      }
    }
  }

  for (fixed yval = zero - tic_step; yval >= y_min; yval -= tic_step) {
    const PixelScalar ymin = ScreenY(yval);
    line[0].y = line[1].y = ymin;

    // STYLE_THINDASHPAPER
    if ((yval > y_min) && (ymin >= rc.top) && (ymin <= rc.bottom - padding_bottom)) {
      canvas.DrawLine(line[0], line[1]);

      if (draw_units) {
        TCHAR unit_text[MAX_PATH];
        FormatTicText(unit_text, yval * unit_step / tic_step, unit_step);

        canvas.text(rc.left + Layout::Scale(8), ymin, unit_text);
      }
    }
  }
}

void
ChartRenderer::ScaleMakeSquare()
{
  if (y_max <= y_min)
    return;

  if (rc.bottom - rc.top - padding_bottom <= 0)
    return;

  fixed ar = fixed(rc.right - rc.left - padding_left)
              / (rc.bottom - rc.top - padding_bottom);
  fixed ard = (x_max - x_min) / (y_max - y_min);
  fixed armod = ard / ar;

  fixed delta;

  if (armod < fixed_one) {
    // need to expand width
    delta = (x_max - x_min) * (fixed_one / armod - fixed_one);
    x_max += delta / 2;
    x_min -= delta / 2;
  } else {
    // need to expand height
    delta = (y_max - y_min) * (armod - fixed_one);
    y_max += delta / 2;
    y_min -= delta / 2;
  }

  // shrink both by 10%
  delta = (x_max - x_min) / 10;
  x_max += delta / 2;
  x_min -= delta / 2;
  delta = (y_max - y_min) / 10;
  y_max += delta / 2;
  y_min -= delta / 2;

  yscale = fixed(rc.bottom - rc.top - padding_bottom) / (y_max - y_min);
  xscale = fixed(rc.right - rc.left - padding_left) / (x_max - x_min);
}

PixelScalar
ChartRenderer::ScreenX(fixed x) const
{
  return (long)((x - x_min) * xscale) + rc.left + padding_left;
}

PixelScalar
ChartRenderer::ScreenY(fixed y) const
{
  return (long)((y_max - y) * yscale) + rc.top;
}

void
ChartRenderer::DrawArrow(const fixed x, const fixed y, const fixed mag,
                         const Angle angle, ChartLook::Style Style)
{
  canvas.Select(look.GetPen(Style));

  RasterPoint wv[2];

  wv[0] = ToScreen(x, y);

  const FastRotation r(angle);
  FastRotation::Pair p;

  p = r.Rotate(mag, fixed_zero);
  wv[1].x = wv[0].x + (int)p.first;
  wv[1].y = wv[0].y + (int)p.second;
  canvas.DrawLine(wv[0], wv[1]);

  p = r.Rotate(mag - fixed(5), fixed(-3));
  wv[1].x = wv[0].x + (int)p.first;
  wv[1].y = wv[0].y + (int)p.second;
  canvas.DrawLine(wv[0], wv[1]);

  p = r.Rotate(mag - fixed(5), fixed(3));
  wv[1].x = wv[0].x + (int)p.first;
  wv[1].y = wv[0].y + (int)p.second;
  canvas.DrawLine(wv[0], wv[1]);
}

void 
ChartRenderer::DrawFilledY(const std::vector<std::pair<fixed, fixed>> &vals,
                           const Brush &brush, const Pen* pen)
{
  if (vals.size()<2)
    return;
  const unsigned fsize = vals.size()+2;
  RasterPoint *line = point_buffer.get(fsize);

  for (unsigned i = 0; i < vals.size(); ++i)
    line[i + 2] = ToScreen(vals[i].first, vals[i].second);

  line[0].x = rc.left + padding_left;
  line[0].y = line[fsize-1].y;
  line[1].x = rc.left + padding_left;
  line[1].y = line[2].y;
  
  canvas.Select(brush);
  if (pen == NULL) {
    canvas.SelectNullPen();
  } else {
    canvas.Select(*pen);
  }
  canvas.DrawPolygon(line, fsize);
}

void
ChartRenderer::DrawDot(const fixed x, const fixed y, const PixelScalar width)
{
  RasterPoint p = ToScreen(x, y);
  RasterPoint line[4] = { { p.x, PixelScalar(p.y - width) },
                          { PixelScalar(p.x - width), p.y },
                          { p.x, PixelScalar(p.y + width) },
                          { PixelScalar(p.x + width), p.y } };
  canvas.SelectNullPen();
  canvas.DrawTriangleFan(line, 4);
}
