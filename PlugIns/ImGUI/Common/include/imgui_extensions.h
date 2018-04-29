
#include "imgui.h"
#include "imgui_internal.h"


namespace ImGui {

  struct Gradient
  {
    virtual ImU32 Calc(const ImVec2& pos) const = 0;
  };

  struct VerticalGradient : public Gradient
  {
    VerticalGradient(const ImVec2& start, const ImVec2& end, const ImVec4& col0, const ImVec4& col1);

    VerticalGradient(const ImVec2& start, const ImVec2& end, ImU32 col0, ImU32 col1);

    void evalStep();

    virtual ImU32 Calc(const ImVec2& pos) const;

    ImVec4 Col0;
    ImVec4 Col1;
    ImVec2 Start;
    ImVec2 End;
    float Len;
  };

  static void AddConvexPolyFilled(ImDrawList* drawList, const ImVec2* points, const int points_count, const Gradient& gradient)
  {
    const ImVec2 uv = drawList->_Data->TexUvWhitePixel;

    if (drawList->Flags & ImDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        drawList->PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = drawList->_VtxCurrentIdx;
        unsigned int vtx_outer_idx = drawList->_VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            drawList->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); drawList->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); drawList->_IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            drawList->_IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2));
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            ImVec2 diff = p1 - p0;
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i0].x = diff.y;
            temp_normals[i0].y = -diff.x;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImU32 col = gradient.Calc(points[i1]);
            const ImU32 col_trans = col & ~IM_COL32_A_MASK;
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            ImVec2 dm = (n0 + n1) * 0.5f;
            float dmr2 = dm.x*dm.x + dm.y*dm.y;
            if (dmr2 > 0.000001f)
            {
                float scale = 1.0f / dmr2;
                if (scale > 100.0f) scale = 100.0f;
                dm *= scale;
            }
            dm *= AA_SIZE * 0.5f;

            // Add vertices
            drawList->_VtxWritePtr[0].pos = (points[i1] - dm); drawList->_VtxWritePtr[0].uv = uv; drawList->_VtxWritePtr[0].col = col;        // Inner
            drawList->_VtxWritePtr[1].pos = (points[i1] + dm); drawList->_VtxWritePtr[1].uv = uv; drawList->_VtxWritePtr[1].col = col_trans;  // Outer
            drawList->_VtxWritePtr += 2;

            // Add indexes for fringes
            drawList->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); drawList->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); drawList->_IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            drawList->_IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); drawList->_IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); drawList->_IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            drawList->_IdxWritePtr += 6;
        }
        drawList->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        drawList->PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            const ImU32 col = gradient.Calc(points[i]);
            drawList->_VtxWritePtr[0].pos = points[i]; drawList->_VtxWritePtr[0].uv = uv; drawList->_VtxWritePtr[0].col = col;
            drawList->_VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            drawList->_IdxWritePtr[0] = (ImDrawIdx)(drawList->_VtxCurrentIdx); drawList->_IdxWritePtr[1] = (ImDrawIdx)(drawList->_VtxCurrentIdx+i-1); drawList->_IdxWritePtr[2] = (ImDrawIdx)(drawList->_VtxCurrentIdx+i);
            drawList->_IdxWritePtr += 3;
        }
        drawList->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
  }

  static inline void PathFillConvex(ImDrawList* drawList, const Gradient& gradient) { AddConvexPolyFilled(drawList, drawList->_Path.Data, drawList->_Path.Size, gradient); drawList->PathClear(); }

}
