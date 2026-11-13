#include "LayoutManager.h"
#include <numeric>
#include <algorithm>

namespace scapms {
namespace gui {

void VBoxLayout::setGeometry(int x, int y, int width, int height) {
    if (m_entries.empty()) return;

    int innerX = x + m_margin;
    int innerY = y + m_margin;
    int innerW = std::max(0, width - 2 * m_margin);
    int innerH = std::max(0, height - 2 * m_margin);

    int totalFixed = 0;
    int totalStretch = 0;
    int visibleCount = 0;

    for (const auto& entry : m_entries) {
        visibleCount++;
        if (entry.fixedHeight > 0) {
            totalFixed += entry.fixedHeight;
        } else {
            totalStretch += std::max(1, entry.stretch);
        }
    }

    int totalSpacing = (visibleCount > 1) ? (visibleCount - 1) * m_spacing : 0;
    int remainingH = std::max(0, innerH - totalFixed - totalSpacing);

    int currentY = innerY;
    for (const auto& entry : m_entries) {
        int itemH = 0;
        if (entry.fixedHeight > 0) {
            itemH = entry.fixedHeight;
        } else if (totalStretch > 0) {
            itemH = (remainingH * std::max(1, entry.stretch)) / totalStretch;
        }

        entry.item->setGeometry(innerX, currentY, innerW, itemH);
        currentY += itemH + m_spacing;
    }
}

void HBoxLayout::setGeometry(int x, int y, int width, int height) {
    if (m_entries.empty()) return;

    int innerX = x + m_margin;
    int innerY = y + m_margin;
    int innerW = std::max(0, width - 2 * m_margin);
    int innerH = std::max(0, height - 2 * m_margin);

    int totalFixed = 0;
    int totalStretch = 0;
    int visibleCount = 0;

    for (const auto& entry : m_entries) {
        visibleCount++;
        if (entry.fixedWidth > 0) {
            totalFixed += entry.fixedWidth;
        } else {
            totalStretch += std::max(1, entry.stretch);
        }
    }

    int totalSpacing = (visibleCount > 1) ? (visibleCount - 1) * m_spacing : 0;
    int remainingW = std::max(0, innerW - totalFixed - totalSpacing);

    int currentX = innerX;
    for (const auto& entry : m_entries) {
        int itemW = 0;
        if (entry.fixedWidth > 0) {
            itemW = entry.fixedWidth;
        } else if (totalStretch > 0) {
            itemW = (remainingW * std::max(1, entry.stretch)) / totalStretch;
        }

        entry.item->setGeometry(currentX, innerY, itemW, innerH);
        currentX += itemW + m_spacing;
    }
}

void GridLayout::setGeometry(int x, int y, int width, int height) {
    if (m_rows <= 0 || m_cols <= 0 || m_cells.empty()) return;

    int innerX = x + m_margin;
    int innerY = y + m_margin;
    int innerW = std::max(0, width - 2 * m_margin);
    int innerH = std::max(0, height - 2 * m_margin);

    int colW = (innerW - (m_cols - 1) * m_spacing) / m_cols;
    int rowH = (innerH - (m_rows - 1) * m_spacing) / m_rows;

    for (const auto& cell : m_cells) {
        int cellX = innerX + cell.col * (colW + m_spacing);
        int cellY = innerY + cell.row * (rowH + m_spacing);
        int cellWidth = cell.colSpan * colW + (cell.colSpan - 1) * m_spacing;
        int cellHeight = cell.rowSpan * rowH + (cell.rowSpan - 1) * m_spacing;

        cell.item->setGeometry(cellX, cellY, cellWidth, cellHeight);
    }
}

} // namespace gui
} // namespace scapms
