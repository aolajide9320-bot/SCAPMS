#ifndef SCAPMS_GUI_LAYOUTMANAGER_H
#define SCAPMS_GUI_LAYOUTMANAGER_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <vector>
#include <memory>
#include <string>

namespace scapms {
namespace gui {

/**
 * @brief Base class for layout management hierarchy (Mandatory Section 8: Layout Management).
 */
class LayoutItem {
public:
    virtual ~LayoutItem() = default;
    virtual void setGeometry(int x, int y, int width, int height) = 0;
};

/**
 * @brief Layout item wrapping a Win32 HWND control.
 */
class WidgetItem : public LayoutItem {
private:
    HWND m_hwnd;

public:
    explicit WidgetItem(HWND hwnd) : m_hwnd(hwnd) {}
    void setGeometry(int x, int y, int width, int height) override {
        if (m_hwnd && IsWindow(m_hwnd)) {
            MoveWindow(m_hwnd, x, y, width, height, TRUE);
        }
    }
    HWND getHWND() const { return m_hwnd; }
};

/**
 * @brief Vertical Box Layout (VBoxLayout) arranges child widgets or sub-layouts vertically.
 */
class VBoxLayout : public LayoutItem {
private:
    struct Entry {
        std::shared_ptr<LayoutItem> item;
        int fixedHeight; // <= 0 means proportional stretch
        int stretch;
    };
    std::vector<Entry> m_entries;
    int m_margin;
    int m_spacing;

public:
    explicit VBoxLayout(int margin = 8, int spacing = 6)
        : m_margin(margin), m_spacing(spacing) {}

    void addWidget(HWND hwnd, int fixedHeight = -1, int stretch = 1) {
        m_entries.push_back({std::make_shared<WidgetItem>(hwnd), fixedHeight, stretch});
    }

    void addLayout(std::shared_ptr<LayoutItem> layout, int fixedHeight = -1, int stretch = 1) {
        m_entries.push_back({layout, fixedHeight, stretch});
    }

    void setGeometry(int x, int y, int width, int height) override;
};

/**
 * @brief Horizontal Box Layout (HBoxLayout) arranges child widgets or sub-layouts horizontally.
 */
class HBoxLayout : public LayoutItem {
private:
    struct Entry {
        std::shared_ptr<LayoutItem> item;
        int fixedWidth; // <= 0 means proportional stretch
        int stretch;
    };
    std::vector<Entry> m_entries;
    int m_margin;
    int m_spacing;

public:
    explicit HBoxLayout(int margin = 8, int spacing = 6)
        : m_margin(margin), m_spacing(spacing) {}

    void addWidget(HWND hwnd, int fixedWidth = -1, int stretch = 1) {
        m_entries.push_back({std::make_shared<WidgetItem>(hwnd), fixedWidth, stretch});
    }

    void addLayout(std::shared_ptr<LayoutItem> layout, int fixedWidth = -1, int stretch = 1) {
        m_entries.push_back({layout, fixedWidth, stretch});
    }

    void setGeometry(int x, int y, int width, int height) override;
};

/**
 * @brief Grid Layout arranges controls in a uniform matrix of rows and columns.
 */
class GridLayout : public LayoutItem {
private:
    struct Cell {
        std::shared_ptr<LayoutItem> item;
        int row;
        int col;
        int rowSpan;
        int colSpan;
    };
    std::vector<Cell> m_cells;
    int m_rows;
    int m_cols;
    int m_margin;
    int m_spacing;

public:
    GridLayout(int rows, int cols, int margin = 8, int spacing = 6)
        : m_rows(rows), m_cols(cols), m_margin(margin), m_spacing(spacing) {}

    void addWidget(HWND hwnd, int row, int col, int rowSpan = 1, int colSpan = 1) {
        m_cells.push_back({std::make_shared<WidgetItem>(hwnd), row, col, rowSpan, colSpan});
    }

    void setGeometry(int x, int y, int width, int height) override;
};

} // namespace gui
} // namespace scapms

#endif // SCAPMS_GUI_LAYOUTMANAGER_H
