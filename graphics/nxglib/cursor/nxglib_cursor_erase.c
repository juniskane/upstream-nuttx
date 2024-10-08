/****************************************************************************
 * graphics/nxglib/cursor/nxglib_cursor_erase.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <assert.h>

#include <nuttx/nx/nxglib.h>

#include "nxglib_bitblit.h"
#include "../nxbe/nxbe.h"
#include "nxglib.h"

#ifdef CONFIG_NX_SWCURSOR

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nxglib_cursor_erase
 *
 * Description:
 *   Erase the cursor region by writing the saved background to the graphics
 *   device memory.
 *
 * Input Parameters:
 *   be      - The back-end state structure instance
 *   bounds  - The region of the display that has been modified.
 *   planeno - The color plane being drawn
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void NXGL_FUNCNAME(nxglib_cursor_erase, NXGLIB_SUFFIX)
(
  FAR struct nxbe_state_s *be,
  FAR const struct nxgl_rect_s *bounds,
  int planeno)
{
  struct nxgl_rect_s intersection;
  struct nxgl_point_s origin;
  FAR struct nxbe_plane_s *plane;
  FAR uint8_t *fbmem;
  FAR const uint8_t *sline;
  FAR uint8_t *dline;
  FAR const NXGL_PIXEL_T *src;
  FAR NXGL_PIXEL_T *dest;
  nxgl_coord_t width;
  nxgl_coord_t height;
  nxgl_coord_t swidth;
  nxgl_coord_t sstride;
  nxgl_coord_t dstride;
  int row;
  int col;

  /* Handle the case some or all of the cursor image is off of the display. */

  nxgl_rectintersect(&intersection, &be->cursor.bounds, &be->bkgd.bounds);

  /* Check if there is anything in the modified region that
   * we need to handle.
   */

  nxgl_rectintersect(&intersection, &intersection, bounds);
  if (!nxgl_nullrect(&intersection))
    {
      /* Get the width and the height of the images to copy in pixels/rows */

      width   = intersection.pt2.x - intersection.pt1.x + 1;
      height  = intersection.pt2.y - intersection.pt1.y + 1;

      /* Get the width of the images in bytes. */

      swidth  = be->cursor.bounds.pt2.x - be->cursor.bounds.pt1.x + 1;
      sstride = NXGL_SCALEX(swidth);

      plane   = &be->plane[planeno];
      dstride = plane->pinfo.stride;

      /* Get the origin position in the background image */

      nxgl_vectsubtract(&origin, &intersection.pt1, &be->cursor.bounds.pt1);

      /* Get the source and destination addresses */

      fbmem  = (FAR uint8_t *)plane->pinfo.fbmem;
      sline  = (FAR uint8_t *)be->cursor.bkgd + sstride * origin.y +
                NXGL_SCALEX(origin.x);
      dline  = (FAR uint8_t *)fbmem + dstride * intersection.pt1.y +
                NXGL_SCALEX(intersection.pt1.x);

      /* Erase the old cursor position by copying the previous content */

      /* Loop for each row */

      for (row = 0; row < height; row++)
        {
          /* Reset to the beginning of the line */

          src  = (FAR NXGL_PIXEL_T *)sline;
          dest = (FAR NXGL_PIXEL_T *)dline;

          /* Loop for each column */

          for (col = 0; col < width; col++)
            {
              *dest++ = *src++;
            }

          /* Update the row addresses to the next row */

          sline += sstride;
          dline += dstride;
        }
    }
}

#endif /* CONFIG_NX_SWCURSOR */
