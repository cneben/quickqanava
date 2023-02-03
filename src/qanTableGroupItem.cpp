/*
 Copyright (c) 2008-2022, Benoit AUTHEMAN All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the author or Destrat.io nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL AUTHOR BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//-----------------------------------------------------------------------------
// This file is a part of the QuickQanava software library.
//
// \file	qanTableGroupItem.cpp
// \author	benoit@destrat.io
// \date	2023 01 26
//-----------------------------------------------------------------------------

// QuickQanava headers
#include "./qanGraph.h"
#include "./qanGroupItem.h"
#include "./qanTableGroupItem.h"

namespace qan { // ::qan

/* TableGroupItem Object Management *///---------------------------------------
TableGroupItem::TableGroupItem(QQuickItem* parent) :
    qan::GroupItem{parent}
{
    setObjectName(QStringLiteral("qan::TableGroupItem"));

    connect(this,   &QQuickItem::widthChanged,
            this,   &TableGroupItem::layoutTable);
    connect(this,   &QQuickItem::heightChanged,
            this,   &TableGroupItem::layoutTable);

    setItemStyle(qan::TableGroup::style(parent));
    setStrictDrop(false);  // Top left corner of a node is enought to allow a drop
}

TableGroupItem::~TableGroupItem()
{
    clearLayout();
}

void    TableGroupItem::componentComplete() { /* Nil */ }

void    TableGroupItem::classBegin() { }

bool    TableGroupItem::setContainer(QQuickItem* container) noexcept
{
    qWarning() << "qan::TableGroupItem::setContainer(): container=" << container;
    if (qan::GroupItem::setContainer(container)) {
        // Note: Force reparenting all borders and cell to container, it might be nullptr
        // at initialization.
        for (const auto verticalBorder: _verticalBorders)
            if (verticalBorder != nullptr)
                verticalBorder->setParentItem(container);
        for (const auto horizontalBorder: _horizontalBorders)
            if (horizontalBorder != nullptr)
                horizontalBorder->setParentItem(container);
        for (const auto cell: _cells)
            if (cell != nullptr)
                cell->setParentItem(container);
        return true;
    }
    return false;
}
//-----------------------------------------------------------------------------


/* Borders and Cells Management *///-------------------------------------------
void    TableGroupItem::clearLayout()
{
    qWarning() << "qan::TableGroupItem::clearLayout()";
    for (const auto verticalBorder: _verticalBorders)
        if (verticalBorder != nullptr)
            verticalBorder->deleteLater();
    _verticalBorders.clear();
    for (const auto horizontalBorder: _horizontalBorders)
        if (horizontalBorder != nullptr)
            horizontalBorder->deleteLater();
    _horizontalBorders.clear();

    for (const auto cell: _cells)
        if (cell != nullptr)
            cell->deleteLater();
    _cells.clear();
}

void    TableGroupItem::initialize(int rows, int cols)
{
    qWarning() << "qan::TableGroupItem::initialize(): rows=" << rows << "  cols=" << cols;
    if (rows <= 0 || cols <= 0) {
        qWarning() << "TableGroupItem::initialize(): Error, invalid rows or cols count.";
        return;
    }

    auto engine = qmlEngine(this);
    if (engine == nullptr) {
        qWarning() << "qan::TableGroupItem::initialize(): Error, no QML engine.";
        return;
    }
    clearLayout();
    createCells(rows * cols);  // Create cells

    auto borderComponent = new QQmlComponent(engine, "qrc:/QuickQanava/TableBorder.qml",
                                             QQmlComponent::PreferSynchronous, nullptr);

    // Notes:
    // - There is no "exterior" borders:
    //    - So there is  cols-1 vertical borders
    //    - And there is rows-1 horizontal borders
    //    - For exemple 6 cells == 4 borders
    // - There is rows*cols cells for (rows-1) + (cols-1) borders.
    //
    // Internal cells vector is indexed row major:
    //   cell1 | cell2 | cell3
    //   ------+-------+------
    //   cell4 | cell5 | cell6
    //   ------+-------+------
    //   cell7 | cell8 | cell9
    //
    // So cell index in _cells at (col=c, row=r) is _cells[(r * cols) + c]

    createBorders(cols - 1, rows - 1);
    int c = 1;
    for (auto verticalBorder: _verticalBorders) {
        if (verticalBorder == nullptr)
            continue;
        for (int r = 0; r < rows; r++) {
            verticalBorder->addPrevCell(_cells[(r * cols) + c - 1]);
            verticalBorder->addNextCell(_cells[(r * cols) + c]);
        }
        c++;
    }

    int r = 1;
    for (auto horizontalBorder: _horizontalBorders) {
        if (horizontalBorder == nullptr)
            continue;
        for (int c = 0; c < cols; c++) {
            horizontalBorder->addPrevCell(_cells[((r-1) * cols) + c]);
            horizontalBorder->addNextCell(_cells[(r * cols)     + c]);
        }
    }

    borderComponent->deleteLater();
}

void    TableGroupItem::createCells(int cellsCount)
{
    if (cellsCount <= 0) {
        qWarning() << "TableGroupItem::createCells(): Error, invalid rows or cols count.";
        return;
    }
    if (cellsCount == static_cast<int>(_cells.size()))
        return;

    auto engine = qmlEngine(this);
    if (engine == nullptr) {
        qWarning() << "qan::TableGroupItem::createCells(): Error, no QML engine.";
        return;
    }

    // Create cells
    auto cellComponent = new QQmlComponent(engine, "qrc:/QuickQanava/TableCell.qml",
                                           QQmlComponent::PreferSynchronous, nullptr);
    for (auto c = 0; c < cellsCount; c++) {
        auto cell = qobject_cast<qan::TableCell*>(createFromComponent(*cellComponent));
        if (cell != nullptr) {
            _cells.push_back(cell);
            cell->setParentItem(getContainer() != nullptr ? getContainer() : this);
            cell->setVisible(true);
        }
    }

    cellComponent->deleteLater();
}

void    TableGroupItem::createBorders(int verticalBordersCount, int horizontalBordersCount)
{
    qWarning() << "qan::TableGroupItem::createBorders(): verticalBordersCount=" << verticalBordersCount << "  horizontalBordersCount=" << horizontalBordersCount;
    if (verticalBordersCount < 0 ||     // Might be 0 for 1x1 tables
        horizontalBordersCount < 0) {
        qWarning() << "TableGroupItem::createBorders(): Error, invalid vertical or horizontal borders count.";
        return;
    }
    auto engine = qmlEngine(this);
    if (engine == nullptr) {
        qWarning() << "qan::TableGroupItem::createBorders(): Error, no QML engine.";
        return;
    }

    auto borderComponent = new QQmlComponent(engine, "qrc:/QuickQanava/TableBorder.qml",
                                             QQmlComponent::PreferSynchronous, nullptr);

    qan::TableBorder* prevBorder = nullptr;
    if (verticalBordersCount != static_cast<int>(_verticalBorders.size())) {
        for (auto v = 0; v < verticalBordersCount; v++) {
            auto border = qobject_cast<qan::TableBorder*>(createFromComponent(*borderComponent));
            if (border != nullptr) {
                border->setTableGroup(getTableGroup());
                border->setOrientation(Qt::Vertical);
                border->setParentItem(getContainer() != nullptr ? getContainer() : this);
                border->setVisible(true);
                border->setPrevBorder(prevBorder);
                connect(border, &qan::TableBorder::modified,
                        this,   [this]() {
                    const auto graph = this->getGraph();
                    const auto tableGroup = this->getTableGroup();
                    if (graph != nullptr &&
                        tableGroup != nullptr)
                    emit graph->tableModified(tableGroup);
                });
                _verticalBorders.push_back(border);

                if (prevBorder != nullptr)  // Audacious initialization of prevBorder nextBorder
                    prevBorder->setNextBorder(border);  // with this border
                prevBorder = border;
            }
        }
    }
    prevBorder = nullptr;
    if (horizontalBordersCount != static_cast<int>(_horizontalBorders.size())) {
        for (auto h = 0; h < horizontalBordersCount; h++) {
            auto border = qobject_cast<qan::TableBorder*>(createFromComponent(*borderComponent));
            if (border != nullptr) {
                border->setTableGroup(getTableGroup());
                border->setOrientation(Qt::Horizontal);
                border->setParentItem(getContainer() != nullptr ? getContainer() : this);
                border->setVisible(true);
                border->setPrevBorder(prevBorder);
                connect(border, &qan::TableBorder::modified,
                        this,   [this]() {
                    const auto graph = this->getGraph();
                    const auto tableGroup = this->getTableGroup();
                    if (graph != nullptr &&
                        tableGroup != nullptr)
                    emit graph->tableModified(tableGroup);
                });
                _horizontalBorders.push_back(border);

                if (prevBorder != nullptr)  // Audacious initialization of prevBorder nextBorder
                    prevBorder->setNextBorder(border);  // with this border
                prevBorder = border;
            }
        }
    }

    borderComponent->deleteLater();
}

auto TableGroupItem::createFromComponent(QQmlComponent& component) -> QQuickItem*
{
    if (!component.isReady()) {
        qWarning() << "qan::TableGroupItem::classBegin(): createTableCell(): Error table cell component is not ready.";
        qWarning() << component.errorString();
        return nullptr;
    }
    const auto rootContext = qmlContext(this);
    if (rootContext == nullptr) {
        qWarning() << "qan::TableGroupItem::classBegin(): createTableCell(): Error, no QML context.";
        return nullptr;
    }
    QQuickItem* item = nullptr;
    QObject* object = component.beginCreate(rootContext);
    if (object == nullptr ||
        component.isError()) {
        if (object != nullptr)
            object->deleteLater();
        return nullptr;
    }
    component.completeCreate();
    if (!component.isError()) {
        QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
        item = qobject_cast<QQuickItem*>(object);
    } // Note: There is no leak until cpp ownership is set
    if (item != nullptr)
        item->setVisible(true);
    return item;
};

void    TableGroupItem::layoutTable()
{
    qWarning() << "qan::TableGroupItem::layoutTable()";
    const auto tableGroup = getTableGroup();
    if (tableGroup == nullptr)
        return;
    const int cols = tableGroup->getCols();
    const int rows = tableGroup->getRows();
    const auto spacing = tableGroup != nullptr ? tableGroup->getCellSpacing() :
                                                 5.;
    const auto padding = tableGroup != nullptr ? tableGroup->getTablePadding() :
                                                 2.;

    if (cols <= 0 || rows <= 0) {
        qWarning() << "qan::TableGroupItem::layoutTable(): Error, rows and columns count can't be <= 0.";
        return;
    }
    if (spacing < 0 || padding < 0) {
        qWarning() << "qan::TableGroupItem::layoutTable(): Error, padding and spacing can't be < 0.";
        return;
    }

    const auto cellWidth = width() > 0. ? (width()
                                           - (2 * padding)
                                           - ((cols - 1) * spacing)) / cols :
                                          0.;
    const auto cellHeight = height() > 0. ? (height()
                                             - (2 * padding)
                                             - ((cols - 1) * spacing)) / rows :
                                            0.;

    //qWarning() << "cellWidth=" << cellWidth;
    //qWarning() << "cellHeight=" << cellHeight;

    if (cellWidth < 0. || cellHeight < 0.) {
        qWarning() << "qan::TableGroupItem::layoutTable(): Error, invalid cell width/height.";
        return;
    }
    // Note: cells are laid out by their borders, do not set their geometry
    // Layout in space

    // Layout vertical borders:
    // |             cell         |         cell         |         cell             |
    // | padding |   cell   |   border  |   cell   |   border  |   cell   | padding |
    //                       <-spacing->            <-spacing->
    if (static_cast<int>(_verticalBorders.size()) == cols - 1) {
        const auto borderWidth = 3.;    // All easy mouse resize handling
        const auto borderWidth2 = borderWidth / 2.;
        for (auto c = 1; c <= (cols - 1); c++) {
            auto* verticalBorder = _verticalBorders[c-1];
            const auto x = padding +
                           ((c - 1) * spacing) +
                           (c * cellWidth) +
                           (spacing / 2.);
            verticalBorder->setX(x - borderWidth2);
            verticalBorder->setY(0.);
            verticalBorder->setWidth(borderWidth);
            verticalBorder->setHeight(height());
        }
    } else
        qWarning() << "qan::TableGoupItem::layoutTable(): Invalid vertical border count.";

    // Layout horizontal borders
    if (static_cast<int>(_horizontalBorders.size()) == rows - 1) {
        const auto borderHeight = 3.;    // All easy mouse resize handling
        const auto borderHeight2 = borderHeight / 2.;
        for (auto r = 1; r <= (rows - 1); r++) {
            auto* horizontalBorder = _horizontalBorders[r-1];
            const auto y = padding +
                           ((r - 1) * spacing) +
                           (r * cellHeight) +
                           (spacing / 2.);
            horizontalBorder->setX(0.);
            horizontalBorder->setY(y - borderHeight2);
            horizontalBorder->setWidth(width());
            horizontalBorder->setHeight(borderHeight);
        }
    } else
        qWarning() << "qan::TableGoupItem::layoutTable(): Invalid horizontal border count.";

    // Note: There is no need to manually call borders layoutCells() method
    // it will be called automatically when border are moved.
}

bool    TableGroupItem::setGroup(qan::Group* group) noexcept
{
    if (qan::GroupItem::setGroup(group)) {
        qWarning() << "TableGroupItem::setGroup(): group=" << group;

        auto tableGroup = qobject_cast<qan::TableGroup*>(group);
        if (tableGroup != nullptr) {
            initialize(tableGroup->getRows(),
                       tableGroup->getCols());

            // Set borders reference to group
            for (auto border: _horizontalBorders)
                if (border)
                    border->setTableGroup(tableGroup);
            for (auto border: _verticalBorders)
                if (border)
                    border->setTableGroup(tableGroup);
            connect(tableGroup, &qan::TableGroup::cellSpacingChanged,
                    this,       &qan::TableGroupItem::layoutTable);
            connect(tableGroup, &qan::TableGroup::cellMinimumSizeChanged,
                    this,       &qan::TableGroupItem::layoutTable);
            connect(tableGroup, &qan::TableGroup::tablePaddingChanged,
                    this,       &qan::TableGroupItem::layoutTable);

            layoutTable();  // Force new layout with actual table group settings
            return true;
        }
    }
    return false;
}

const qan::TableGroup*  TableGroupItem::getTableGroup() const { return qobject_cast<const qan::TableGroup*>(getGroup()); }
qan::TableGroup*        TableGroupItem::getTableGroup() { return qobject_cast<qan::TableGroup*>(getGroup()); };
//-----------------------------------------------------------------------------


/* TableGroupItem DnD Management *///------------------------------------------
void    TableGroupItem::groupNodeItem(qan::NodeItem* nodeItem, bool transform)
{
    // PRECONDITIONS:
        // nodeItem can't be nullptr
        // A 'container' must have been configured
    if (nodeItem == nullptr ||
        getContainer() == nullptr)   // A container must have configured in concrete QML group component
        return;

    // Note: no need for the container to be visible or open.
    auto groupPos = QPointF{nodeItem->x(), nodeItem->y()};
    if (transform) {
        const auto globalPos = nodeItem->mapToGlobal(QPointF{0., 0.});
        groupPos = getContainer()->mapFromGlobal(globalPos);
        // Find cell at groupPos and attach node to cell
        for (const auto& cell: _cells) {
            const auto cellBr = cell->boundingRect().translated(cell->position());
            if (cellBr.contains(groupPos)) {
                cell->setItem(nodeItem);
                nodeItem->getNode()->setCell(cell);
                break;
            }
        }
    }
    groupMoved();           // Force call to groupMoved() to update group adjacent edges
    endProposeNodeDrop();
}

void    TableGroupItem::ungroupNodeItem(qan::NodeItem* nodeItem, bool transform)
{
    if (nodeItem == nullptr)   // A container must have configured in concrete QML group component
        return;
    if (getGraph() &&
        getGraph()->getContainerItem() != nullptr) {
        const QPointF nodeGlobalPos = nodeItem->mapToItem(getGraph()->getContainerItem(),
                                                          QPointF{0., 0.});
        nodeItem->setParentItem(getGraph()->getContainerItem());
        if (transform)
            nodeItem->setPosition(nodeGlobalPos + QPointF{10., 10.});  // A delta to visualize ungroup
        nodeItem->setZ(z() + 1.);
        nodeItem->setDraggable(true);
        nodeItem->setDroppable(true);
        nodeItem->setSelectable(true);
        nodeItem->getNode()->setCell(nullptr);
    }
}

void    TableGroupItem::mouseDoubleClickEvent(QMouseEvent* event)
{
    qan::NodeItem::mouseDoubleClickEvent(event);
    if (event->button() == Qt::LeftButton &&
        (getNode() != nullptr &&
         !getNode()->getLocked()))
        emit groupDoubleClicked(this, event->localPos());
}

void    TableGroupItem::mousePressEvent(QMouseEvent* event)
{
    qan::NodeItem::mousePressEvent(event);

    if (event->button() == Qt::LeftButton &&    // Selection management
         getGroup() &&
         isSelectable() &&
         !getCollapsed() &&         // Locked/Collapsed group is not selectable
         !getNode()->getLocked()) {
        if (getGraph())
            getGraph()->selectGroup(*getGroup(), event->modifiers());
    }

    if (event->button() == Qt::LeftButton)
        emit groupClicked(this, event->localPos());
    else if (event->button() == Qt::RightButton)
        emit groupRightClicked(this, event->localPos());
}
//-----------------------------------------------------------------------------

} // ::qan