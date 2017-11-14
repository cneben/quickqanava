/*
 Copyright (c) 2008-2017, Benoit AUTHEMAN All rights reserved.

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
// \file	qanEdgeItem.h
// \author	benoit@destrat.io
// \date	2016 03 04
//-----------------------------------------------------------------------------

#ifndef qanEdgeItem_h
#define qanEdgeItem_h

// Qt headers
#include <QLineF>

// QuickQanava headers
#include "./qanGraphConfig.h"
#include "./qanStyle.h"
#include "./qanNodeItem.h"
#include "./qanNode.h"

namespace qan { // ::qan

class Graph;
class Edge;
class NodeItem;

//! Weighted directed edge linking two nodes in a graph.
/*!
 *
 * \warning EdgeItem \c objectName property is set to "qan::EdgeItem" and should not be changed in subclasses.
 *
 * \nosubgrouping
 */
class EdgeItem : public QQuickItem
{
    /*! \name Edge Object Management *///--------------------------------------
    //@{
    Q_OBJECT
public:
    explicit EdgeItem(QQuickItem* parent = nullptr);
    virtual ~EdgeItem();
    EdgeItem( const EdgeItem& ) = delete;

public:
    Q_PROPERTY( qan::Edge* edge READ getEdge CONSTANT FINAL )
    auto        getEdge() noexcept -> qan::Edge*;
    auto        getEdge() const noexcept -> const qan::Edge*;
    auto        setEdge(qan::Edge* edge) noexcept -> void;
private:
    QPointer<qan::Edge>    _edge;

public:
    Q_PROPERTY( qan::Graph* graph READ getGraph WRITE setGraph NOTIFY graphChanged )
    //! Secure shortcut to getEdge().getGraph().
    auto    getGraph() const noexcept -> const qan::Graph*;
    //! \copydoc getGraph()
    auto    getGraph() noexcept -> qan::Graph*;
    auto    setGraph(qan::Graph*) noexcept -> void;
signals:
    void    graphChanged();
private:
    QPointer<qan::Graph> _graph;
    //@}
    //-------------------------------------------------------------------------

    /*! \name Edge Topology Management *///------------------------------------
    //@{
public:
    Q_INVOKABLE bool        isHyperEdge() const noexcept;
public:
    Q_PROPERTY( qan::NodeItem* sourceItem READ getSourceItem WRITE setSourceItem NOTIFY sourceItemChanged FINAL )
    qan::NodeItem*          getSourceItem( ) { return _sourceItem.data(); }
    void                    setSourceItem( qan::NodeItem* source );
private:
    QPointer<qan::NodeItem> _sourceItem;
signals:
    void                    sourceItemChanged( );

public:
    Q_PROPERTY( qan::NodeItem* destinationItem READ getDestinationItem WRITE setDestinationItem NOTIFY destinationItemChanged FINAL )
    qan::NodeItem*          getDestinationItem( ) noexcept { return _destinationItem.data(); }
    void                    setDestinationItem( qan::NodeItem* destination );
private:
    QPointer<qan::NodeItem> _destinationItem;
signals:
    void                    destinationItemChanged( );

public:
    Q_PROPERTY( qan::EdgeItem* destinationEdge READ getDestinationEdge() WRITE setDestinationEdge NOTIFY destinationEdgeChanged FINAL )
    qan::EdgeItem*  getDestinationEdge() noexcept { return _destinationEdge.data(); }
    void            setDestinationEdge( qan::EdgeItem* destination );
signals:
    void            destinationEdgeChanged( );
private:
    QPointer<qan::EdgeItem> _destinationEdge;
protected:
    //! Configure either a node or an edge (for hyper edges) item.
    void            configureDestinationItem( QQuickItem* item );
    //@}
    //-------------------------------------------------------------------------

    /*! \name Edge Drawing Management *///-------------------------------------
    //@{
public:
    /*! Hidden is set to true when the edge \i should not be shown, it is up to the user to use thie property to eventually hide the item.
     *
     *  \c hidden property is automatically set to true when either the edge is inside source or destination bounding box or the line is
     *  too short to be drawn.
     */
    Q_PROPERTY( bool hidden READ getHidden() NOTIFY hiddenChanged FINAL )
    inline bool getHidden() const noexcept { return _hidden; }
    void        setHidden(bool hidden) noexcept;
signals:
    void        hiddenChanged( );
private:
    bool        _hidden{false};

public slots:
    //! Call updateItem() (override updateItem() to an empty method for invisible edges).
    virtual void        updateItemSlot( ) { updateItem(); }
public:
    /*! \brief Update edge bounding box according to source and destination item actual position and size.
     *
     * \note When overriding, call base implementation at the beginning of user implementation.
     * \note Override to an empty method with no base class calls for an edge with no graphics content.
     */
    virtual void        updateItem() noexcept;
protected:
    void                updateStraightItem() noexcept;
    void                updateArrowGeometry() noexcept;
    /*! Return line angle on line \c line.
     *
     * \return angle in degree or a value < 0.0 if an error occurs.
     */
    qreal               lineAngle(const QLineF& line) const noexcept;
public:
    //! Internally used from QML to set src and dst and display an unitialized edge for previewing edges styles.
    Q_INVOKABLE void    setLine( QPoint src, QPoint dst );
    //! Edge source point in item CS (with accurate source bounding shape intersection).
    Q_PROPERTY( QPointF p1 READ getP1() NOTIFY lineGeometryChanged FINAL )
    inline  auto    getP1() const noexcept -> const QPointF& { return _p1; }
    //! Edge destination point in item CS (with accurate destination bounding shape intersection).
    Q_PROPERTY( QPointF p2 READ getP2() NOTIFY lineGeometryChanged FINAL )
    inline  auto    getP2() const noexcept -> const QPointF& { return _p2; }
signals:
    void            lineGeometryChanged();
protected:
    QPointF         _p1;
    QPointF         _p2;
protected:
    QPointF         getLineIntersection( const QPointF& p1, const QPointF& p2, const QPolygonF& polygon ) const noexcept;
    QLineF          getLineIntersection( const QPointF& p1, const QPointF& p2, const QPolygonF& srcBp, const QPolygonF& dstBp ) const noexcept;
    //@}
    //-------------------------------------------------------------------------

    /*! \name Curve Control Points Management *///-----------------------------
    //@{
protected:
    /*! \brief Call qan::EdgeItem::updateItem(), then generate control points specific for curve edge.
     *
     * \copydoc qan::EdgeITem::updateItem()
     */
    void            updateCurvedItem() noexcept;
public:
    //! Edge source point in item CS (with accurate source bounding shape intersection).
    Q_PROPERTY( QPointF c1 READ getC1() NOTIFY c1Changed FINAL )
    //! \copydoc c1
    inline  auto    getC1() const noexcept -> const QPointF& { return _c1; }
    //! Edge destination point in item CS (with accurate destination bounding shape intersection).
    Q_PROPERTY( QPointF c2 READ getC2() NOTIFY c2Changed FINAL )
    //! \copydoc c2
    inline  auto    getC2() const noexcept -> const QPointF& { return _c2; }
signals:
    //! \copydoc c1
    void            c1Changed();
    //! \copydoc c2
    void            c2Changed();
private:
    //! \copydoc c1
    QPointF         _c1;
    //! \copydoc c2
    QPointF         _c2;

protected:
    /*! Return cubic curve angle at position \c pos between [0.; 1.] on curve defined by \c start, \c end and controls points \c c1 and \c c2.
     *
     * \param  pos  linear position on curve, between [0.; 1.0].
     * \return angle in degree or a value < 0.0 if an error occurs.
     */
    qreal           cubicCurveAngleAt(qreal pos, const QPointF& start, const QPointF& end, const QPointF& c1, const QPointF& c2) const noexcept;

public:
    //! Destination edge arrow angle.
    Q_PROPERTY( qreal dstAngle READ getDstAngle() NOTIFY dstAngleChanged FINAL )
    //! \copydoc dstAngle
    inline  auto    getDstAngle() const noexcept -> qreal { return _dstAngle; }
private:
    //! \copydoc dstAngle
    qreal           _dstAngle{0.};
signals:
    //! \copydoc dstAngle
    void            dstAngleChanged();

public:
    //! Source edge arrow angle.
    Q_PROPERTY( qreal srcAngle READ getSrcAngle() NOTIFY srcAngleChanged FINAL )
    //! \copydoc srcAngle
    inline  auto    getSrcAngle() const noexcept -> qreal { return _srcAngle; }
private:
    //! \copydoc srcAngle
    qreal           _srcAngle{0.};
signals:
    //! \copydoc srcAngle
    void            srcAngleChanged();

public:
    /*! \brief Edge destination arrow control points (\c dstA1 is top corner, \c dstA2 is tip, \c dstA3 is bottom corner).
     *
     * \note Arrow geometry is updated with a single arrowGeometryChanged() to avoid unecessary binding: all points
     * geometry must be changed at the same time.
     */
    Q_PROPERTY( QPointF dstA1 READ getDstA1() NOTIFY arrowGeometryChanged FINAL )
    //! \copydoc dstA1
    inline  auto    getDstA1() const noexcept -> const QPointF& { return _dstA1; }
    //! \copydoc dstA1
    Q_PROPERTY( QPointF dstA2 READ getDstA2() NOTIFY arrowGeometryChanged FINAL )
    //! \copydoc dstA1
    inline  auto    getDstA2() const noexcept -> const QPointF& { return _dstA2; }
    //! \copydoc dstA1
    Q_PROPERTY( QPointF dstA3 READ getDstA3() NOTIFY arrowGeometryChanged FINAL )
    //! \copydoc dstA1
    inline  auto    getDstA3() const noexcept -> const QPointF& { return _dstA3; }
private:
    //! \copydoc dstA1
    QPointF         _dstA1, _dstA2, _dstA3;
signals:
    void            arrowGeometryChanged();
    //@}
    //-------------------------------------------------------------------------

    /*! \name Mouse Management *///--------------------------------------------
    //@{
protected:
    virtual void    mouseDoubleClickEvent( QMouseEvent* event ) override;
    virtual void    mousePressEvent( QMouseEvent* event ) override;
signals:
    void            edgeClicked( qan::EdgeItem* edge, QPointF pos );
    void            edgeRightClicked( qan::EdgeItem* edge, QPointF pos );
    void            edgeDoubleClicked( qan::EdgeItem* edge, QPointF pos );
private:
    inline qreal    distanceFromLine( const QPointF& p, const QLineF& line ) const;

public:
    //! Edge label position.
    Q_PROPERTY( QPointF labelPos READ getLabelPos WRITE setLabelPos NOTIFY labelPosChanged FINAL )
    //! Get edge label position.
    QPointF		getLabelPos( ) const { return _labelPos; }
    //! Set edge label position.
    void		setLabelPos( QPointF labelPos ) { _labelPos = labelPos; emit labelPosChanged(); }
protected:
    //! \sa labelPos
    QPointF     _labelPos;
signals:
    //! \sa labelPos
    void        labelPosChanged( );
    //@}
    //-------------------------------------------------------------------------

    /*! \name Style and Properties Management *///-----------------------------
    //@{
public:
    //! Edge current style object (this property is never null, a default style is returned when no style has been manually set).
    Q_PROPERTY( qan::EdgeStyle* style READ getStyle WRITE setStyle NOTIFY styleChanged FINAL )
    void            setStyle( EdgeStyle* style ) noexcept;
    qan::EdgeStyle* getStyle() const noexcept { return _style.data(); }
private:
    QPointer<qan::EdgeStyle>    _style{nullptr};
signals:
    void            styleChanged();
private slots:
    //! Called when the style associed to this edge is destroyed.
    void            styleDestroyed( QObject* style );
    //@}
    //-------------------------------------------------------------------------

    /*! \name Drag'nDrop Management  *///--------------------------------------
    //@{
public:
    /*! \brief Define if the edge actually accept drops.
     *
     * When set to false, the edge will not be styleable by DnD and hyper edge drop connector
     * won't works.
     *
     * Default to true.
     *
     * Setting this property to false may lead to a significant performance improvement if DropNode support is not needed.
     */
    Q_PROPERTY( bool acceptDrops READ getAcceptDrops WRITE setAcceptDrops NOTIFY acceptDropsChanged FINAL )
    void             setAcceptDrops( bool acceptDrops ) { _acceptDrops = acceptDrops; setFlag( QQuickItem::ItemAcceptsDrops, acceptDrops ); emit acceptDropsChanged( ); }
    bool             getAcceptDrops( ) { return _acceptDrops; }
private:
    bool            _acceptDrops{true};
signals:
    void            acceptDropsChanged( );

protected:
    //! Return true if point is actually on the edge (not only in edge bounding rect).
    virtual bool    contains( const QPointF& point ) const override;

    /*! \brief Internally used to manage drag and drop over nodes, override with caution, and call base class implementation.
     *
     * Drag enter event are not restricted to the edge bounding rect but to the edge line with a distance delta, computing
     * that value is quite coslty, if you don't need to accept drops, use setAcceptDrops( false ).
     */
    virtual void    dragEnterEvent( QDragEnterEvent* event ) override;
    //! Internally used to manage drag and drop over nodes, override with caution, and call base class implementation.
    virtual void    dragMoveEvent( QDragMoveEvent* event ) override;
    //! Internally used to manage drag and drop over nodes, override with caution, and call base class implementation.
    virtual void    dragLeaveEvent( QDragLeaveEvent* event ) override;
    //! Internally used to accept style drops.
    virtual void    dropEvent( QDropEvent* event ) override;
    //@}
    //-------------------------------------------------------------------------
};

} // ::qan

QML_DECLARE_TYPE( qan::EdgeItem )

#endif // qanEdgeItem_h
