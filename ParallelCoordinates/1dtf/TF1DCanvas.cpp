#include "tf1dcanvas.h"
#include <QColor>
#include <QAction>
#include <qstring.h>
#include <QString>
#include <QPainter>
#include <QToolTip>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QColorDialog>
#include <QTextStream>
#include <sstream>
#include "1dtf/TF1D.h"
#include "1dtf/HistogramPainter.h"

#define  max(a,b) (a)>(b) ? (a) : (b)
#define  min(a,b) (a)<(b) ? (a) : (b)

TF1DCanvas::TF1DCanvas(QWidget *parent)
: QWidget(parent)
, m_iMaximumIntensity(255)
, m_bIsNoColor(false)
, m_xRange(0.f, 1.f), m_yRange(0.f, 1.f), m_xAxisText(tr("intensity")), m_yAxisText(tr("opacity"))
{
	m_iPadding = 12;
	m_iBottomSpace = 42;
	m_iArrowLength = 10;
	m_iArrowWidth = 3;
	m_iPointSize = 9;
	m_pSelectedKey = 0;
	m_iMinCellSize = 8;
	m_bIsDragging = false;
	m_iDragLine = -1;
	m_fDragLineAlphaLeft = -1.f;
	m_fDragLineAlphaRight = -1.f;
	m_bIsDrawHist = false;
	m_bIsChanged = false;

	m_keyColletion = new TFMappingKeyCollection();
	m_pTransfunc = new TF1D();

	m_pHistogramPainter = new HistogramPainter(this, m_xRange, m_yRange, m_iPadding, m_iPadding, m_iBottomSpace, m_iPadding, m_iArrowLength);
	m_pHistogramPainter->setHisStyle(1);
	m_pHistogramPainter->installEventFilter(this);

	setObjectName("TF1DCanvas");
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);

	setFocus();

	setThreshold(0.f, 1.f);
	
	//send the tf to renderwidget
	setupTransfunc();
}

TF1DCanvas::~TF1DCanvas()
{

}

void TF1DCanvas::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	m_pHistogramPainter->resize(width(), height());
	m_gridSpacing = Vector2f(m_xRange.y, m_yRange.y);
	// refine gridSpacing_ as good as possible
	Vector2f cellSize = wtos(m_gridSpacing) - wtos(Vector2f(m_xRange.x, m_yRange.x));
	//计算格子数目
	m_gridSpacingNum.x = cellSize.x * (0.04);
	m_gridSpacingNum.y = cellSize.y * (0.04);
}

void TF1DCanvas::paintEvent(QPaintEvent* event)
{
	if (!m_keyColletion)
		return;
	event->accept();
	QPainter paint(this);

	QMatrix m;
	m.translate(0.0, static_cast<float>(height())-1);
	m.scale(1.f, -1.f);
	paint.setMatrix(m);

	paint.setMatrixEnabled(true);
	paint.setRenderHint(QPainter::Antialiasing, false);
	paint.setPen(Qt::NoPen);
	paint.setBrush(Qt::white);
	paint.drawRect(0, 0, width() - 1, height() - 1);

	// draw grid
	paint.setPen(QColor(220, 220, 220));
	paint.setRenderHint(QPainter::Antialiasing, false);

	Vector2f pmin = Vector2f(m_xRange.x, m_yRange.x); //相对坐标的范围
	Vector2f pmax = Vector2f(m_xRange.y, m_yRange.y);
	int xgridNum = m_gridSpacingNum.x;
	int ygridNum = m_gridSpacingNum.y;
	m_gridSpacing.x = (pmax.x - pmin.x) / xgridNum;
	m_gridSpacing.y = (pmax.y - pmin.y) / ygridNum;

	int num = 0;
	for (float f=pmin.x; f<pmax.x+m_gridSpacing.x*0.5; f+=m_gridSpacing.x, ++num) {
		Vector2f p = wtos(Vector2f(f, 0.f));
		Vector2f a = wtos(Vector2f(0.f, 0.f));
		Vector2f b = wtos(Vector2f(0.f, 1.f));
		paint.drawLine(QPointF(p.x, a.y),QPointF(p.x, b.y));

		paint.scale(-1.f, 1.f);
		paint.rotate(180.f);
		QString strval = QString::number((int)(/*vxGlobalProperty::getIntensityMax()*/m_iMaximumIntensity * ((float)num/xgridNum)));
		paint.setPen(Qt::darkRed);
		if (num % 2 == 0)
			paint.drawText(static_cast<int>(p.x - 10), static_cast<int>(-0.7 * (m_iBottomSpace)),  strval);
		else
			paint.drawText(static_cast<int>(p.x - 10), static_cast<int>(-0.45 * (m_iBottomSpace)),  strval);
		paint.setPen(QColor(220, 220, 220));
		paint.rotate(180.f);
		paint.scale(-1.f, 1.f);
	}

	for (float f=pmin.y; f<pmax.y+m_gridSpacing.y*0.5; f+=m_gridSpacing.y) {
		Vector2f p = wtos(Vector2f(0.f, f));
		Vector2f a = wtos(Vector2f(0.f, 0.f));
		Vector2f b = wtos(Vector2f(1.f, 0.f));
		paint.drawLine(QPointF(a.x, p.y),
			QPointF(b.x, p.y));
	}

	// draw x and y axes
	paint.setRenderHint(QPainter::Antialiasing, true);
	paint.setPen(Qt::gray);
	paint.setBrush(Qt::gray);

	// draw axes independently from visible range
	float oldx0_ = m_xRange[0];
	float oldx1_ = m_xRange[1];
	m_xRange[0] = 0.f;
	m_xRange[1] = 1.f;

	Vector2f origin = wtos(Vector2f(0.f, 0.f));
	origin.x = floor(origin.x) + 0.5f;
	origin.y = floor(origin.y) + 0.5f;

	paint.setRenderHint(QPainter::Antialiasing, true);

	paint.drawLine(QPointF(m_iPadding, origin.y),
		QPointF(width() - m_iPadding, origin.y));

	paint.drawLine(QPointF(origin.x, m_iBottomSpace),
		QPointF(origin.x, height() - m_iPadding));

	// draw arrows
	QPointF arrow[3];
	arrow[0] = QPointF(origin.x, height() - m_iPadding);
	arrow[1] = QPointF(origin.x + m_iArrowWidth, height() - m_iPadding - m_iArrowLength);
	arrow[2] = QPointF(origin.x - m_iArrowWidth, height() - m_iPadding - m_iArrowLength);

	paint.drawConvexPolygon(arrow, 3);

	arrow[0] = QPointF(width() - m_iPadding, origin.y);
	arrow[1] = QPointF(width() - m_iPadding - m_iArrowLength, origin.y - m_iArrowWidth);
	arrow[2] = QPointF(width() - m_iPadding - m_iArrowLength, origin.y + m_iArrowWidth);

	paint.drawConvexPolygon(arrow, 3);

	paint.scale(-1.f, 1.f);
	paint.rotate(180.f);
	paint.drawText(static_cast<int>(width() - 6.2f * m_iPadding), static_cast<int>(-1 * (origin.y - 0.9 * m_iBottomSpace)), m_xAxisText);
	paint.drawText(static_cast<int>(1.6f * m_iPadding), static_cast<int>(-1 * (height() - 1.85f * m_iPadding)), m_yAxisText);

	paint.rotate(180.f);
	paint.scale(-1.f, 1.f);

	m_xRange[0] = oldx0_;
	m_xRange[1] = oldx1_;

	// draw mapping function
	QPen pen = QPen(Qt::darkRed);
	pen.setWidthF(1.5f);
	paint.setPen(pen);

	origin = wtos(Vector2f(0.f));

	Vector2f old;
	for (int i=0; i < m_keyColletion->getNumKeys(); ++i) {
		TFMappingKey *key = m_keyColletion->getKey(i);
		Vector2f p = wtos(Vector2f(key->getIntensity(), key->getColor().a / 255.f));
		if (i == 0)  {
			if (m_keyColletion->getKey(0)->getIntensity() > 0.f)
			{
				paint.drawLine(QPointF(wtos(Vector2f(0.f, 0.f)).x, p.y), QPointF(p.x - 1.f, p.y));								
			}
		} else {
			paint.drawLine(QPointF(old.x + 1.f, old.y),QPointF(p.x - 1.f, p.y));				
		}
		old = p;
	}
	if (m_keyColletion->getKey(m_keyColletion->getNumKeys()-1)->getIntensity() < 1.f) {
		paint.drawLine(QPointF(old.x + 1.f, old.y),QPointF(wtos(Vector2f(1.f, 0.f)).x, old.y));			
	}

	if (m_xRange[1] != m_xRange[0])
		paintKeys(paint);

	//threshold area
	paint.setBrush(QBrush(QColor(192, 192, 192, 230), Qt::SolidPattern));
	paint.setPen(Qt::NoPen);
	Vector2f upperRight = wtos(Vector2f(1.f));
	Vector2f lowerLeft = wtos(Vector2f(0.f));
	int w = static_cast<int>(upperRight.x - lowerLeft.x);
	int h = static_cast<int>(upperRight.y - lowerLeft.y);

	if (m_fThresholdL > 0.f) {
		paint.drawRect(static_cast<int>(origin.x), static_cast<int>(origin.y),
			static_cast<int>(m_fThresholdL * w + 1), h);
	}
	if (m_fThresholdU < 1.f) {
		paint.drawRect(static_cast<int>(origin.x + floor(m_fThresholdU * w)),
			static_cast<int>(origin.y), static_cast<int>((1 - m_fThresholdU) * w + 1), h);
	}

	paint.setRenderHint(QPainter::Antialiasing, false);

	paint.setPen(Qt::lightGray);
	paint.setBrush(Qt::NoBrush);
	paint.drawRect(0, 0, width() - 1, height() - 1);

	paint.setMatrixEnabled(false);

	setupTransfunc();
}

void TF1DCanvas::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		emit toggleInteractionMode(true);

	event->accept();

	m_iDragLine = hitLine(Vector2f(event->x(), event->y()));
	if (m_iDragLine >= 0 && event->modifiers() == Qt::ShiftModifier) {
		m_iDragLineStartY = event->y();
		return;
	}

	Vector2f sHit = Vector2f(event->x(), static_cast<float>(height()) - event->y());
	Vector2f hit = stow(sHit);

	// see if a key was selected
	m_pSelectedKey = 0;
	for (int i=0; i < m_keyColletion->getNumKeys(); ++i) {
		TFMappingKey* key = m_keyColletion->getKey(i);
		Vector2f sp = wtos(Vector2f(key->getIntensity(), key->getColor().a / 255.0));

		if (sHit.x > sp.x - m_iPointSize && sHit.x < sp.x + m_iPointSize &&
			sHit.y > sp.y - m_iPointSize && sHit.y < sp.y + m_iPointSize) {
				m_pSelectedKey = key;
		}
	}

	//移动Key
	if (m_pSelectedKey != 0 && event->button() == Qt::LeftButton) {
		m_bIsDragging = true;
		//keep values within valid range
		hit.x = max(hit.x,0.f);
		hit.x = min(hit.x,1.f);
		hit.y = max(hit.y,0.f);
		hit.y = min(hit.y,1.f);

		updateCoordinates(event->pos(), hit);
		emit colorChanged(Col2QColor(m_pSelectedKey->getColor()));
		return;
	}

	//加入新的Key
	if (hit.x >= 0.f && hit.x <= 1.f && hit.y >= 0.f && hit.y <= 1.f && event->button() == Qt::LeftButton) {
		insertNewKey(hit);
		m_bIsDragging = true;
		m_iDragLine = -1;
		updateCoordinates(event->pos(), hit);
		update();
		emit colorChanged(Col2QColor(m_pSelectedKey->getColor()));
		emit changed();
	}
}

void TF1DCanvas::mouseMoveEvent(QMouseEvent* event)
{
	event->accept();
	m_mousePos = event->pos();

	Vector2f sHit = Vector2f(event->x(), static_cast<float>(height()) - event->y());
	Vector2f hit = stow(sHit);


	if (!m_bIsDragging && hitLine(Vector2f(event->x(), event->y())) >= 0 && event->modifiers() == Qt::ShiftModifier)
		setCursor(Qt::SizeVerCursor);
	else
		unsetCursor();

	if (m_iDragLine >= 0) {
		// a line between 2 keys is moved (shift modifier was used)
		float delta = m_iDragLineStartY - event->y();
		m_iDragLineStartY = event->y();
		//left key
		TFMappingKey* key = m_keyColletion->getKey(m_iDragLine);
		if (m_fDragLineAlphaLeft == -1.f)
			m_fDragLineAlphaLeft = key->getAlpha();
		m_fDragLineAlphaLeft = wtos(Vector2f(m_fDragLineAlphaLeft)).y;
		m_fDragLineAlphaLeft += delta;
		m_fDragLineAlphaLeft = stow(Vector2f(m_fDragLineAlphaLeft)).y;
		if (m_fDragLineAlphaLeft < 0.f)
			m_fDragLineAlphaLeft = 0.f;
		if (m_fDragLineAlphaLeft > 1.f)
			m_fDragLineAlphaLeft = 1.f;
		key->setAlpha(m_fDragLineAlphaLeft);
		m_keyColletion->updateKey(key);
		if (m_keyColletion->getNumKeys() >= m_iDragLine+1) {
			//right key - when existing
			key = m_keyColletion->getKey(m_iDragLine+1);
			if (m_fDragLineAlphaRight == -1.f)
				m_fDragLineAlphaRight = key->getAlpha();
			m_fDragLineAlphaRight = wtos(Vector2f(m_fDragLineAlphaRight)).y;
			m_fDragLineAlphaRight += delta;
			m_fDragLineAlphaRight = stow(Vector2f(m_fDragLineAlphaRight)).y;
			if (m_fDragLineAlphaRight < 0.f)
				m_fDragLineAlphaRight = 0.f;
			if (m_fDragLineAlphaRight > 1.f)
				m_fDragLineAlphaRight = 1.f;
			key->setAlpha(m_fDragLineAlphaRight);
			m_keyColletion->updateKey(key);
		}
		update();
		emit changed();
		return;
	}

	// return when no key was inserted or selected
	if (!m_bIsDragging)
		return;

	// keep location within valid texture coord range
	hit.x = max(hit.x,0.f);
	hit.x = min(hit.x,1.f);
	hit.y = max(hit.y,0.f);
	hit.y = min(hit.y,1.f);

	if (m_pSelectedKey != 0) {
		updateCoordinates(event->pos(), hit);
		if (event->modifiers() != Qt::ShiftModifier) {
			m_pSelectedKey->setIntensity(hit.x);
		}
		if (event->modifiers() != Qt::ControlModifier) {
			m_pSelectedKey->setAlpha(hit.y);
		}
		bool selectedFound = false;
		for (size_t i = 0; i < m_keyColletion->getNumKeys(); ++i) {
			TFMappingKey* key = m_keyColletion->getKey(static_cast<int>(i));
			//is the tf key the selected one?
			if (key == m_pSelectedKey) {
				selectedFound = true;
				continue;
			}
			if (selectedFound) {
				//change intensity of key if its lower than the intensity of selectedKey_
				if (key->getIntensity() < m_pSelectedKey->getIntensity())
					key->setIntensity(m_pSelectedKey->getIntensity());
			} else {
				//change intensity of key if its higher than the intensity of selectedKey_
				if (key->getIntensity() > m_pSelectedKey->getIntensity())
					key->setIntensity(m_pSelectedKey->getIntensity());
			}
		}
		m_keyColletion->updateKey(m_pSelectedKey);

		update();
		emit changed();
	}
}

void TF1DCanvas::mouseReleaseEvent(QMouseEvent* event)
{
	event->accept();
	if (event->button() == Qt::LeftButton) {
		m_bIsDragging = false;
		m_iDragLine = -1;
		m_fDragLineAlphaLeft = -1.f;
		m_fDragLineAlphaRight = -1.f;
		hideCoordinates();
		update();
		emit toggleInteractionMode(false);
	}
}

void TF1DCanvas::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Shift  && underMouse() && hitLine(Vector2f(m_mousePos.x(), m_mousePos.y())) >= 0 && !m_bIsDragging) {
		setCursor(Qt::SizeVerCursor);
	}
}

void TF1DCanvas::keyReleaseEvent(QKeyEvent* event)
{
	unsetCursor();
	if (event->key() == Qt::Key_Delete && m_pSelectedKey != 0) {
		event->accept();
		deleteKey();
	}
}

void TF1DCanvas::mouseDoubleClickEvent(QMouseEvent *event) {
	event->accept();
	if (event->button() == Qt::LeftButton)
		changeCurrentColor();
}

bool TF1DCanvas::eventFilter( QObject *obj, QEvent *e )
{
	if (e->type() == QEvent::MouseMove)
	{
		mouseMoveEvent(static_cast<QMouseEvent*>(e));
	}

	// pass the event on to the parent class
	return QWidget::eventFilter(obj, e);
}

void TF1DCanvas::changeCurrentColor(const QColor& c)
{
	if (!m_pSelectedKey || !c.isValid())
		return;

	Color4 voxercolor = QColor2Col(c);
	bool changedColor = false;

	voxercolor.a = m_pSelectedKey->getColor().a;
	if (m_pSelectedKey->getColor() != voxercolor) {
		m_pSelectedKey->setColor(voxercolor);
		changedColor = true;
	}

	if (changedColor) {
		update();
		emit changed();
		emit colorChanged(c);
	}
}

void TF1DCanvas::deleteKey()
{
	if (!m_pSelectedKey || m_keyColletion->getNumKeys() < 3)
		return;

	m_keyColletion->removeKey(m_pSelectedKey);
	m_pSelectedKey = 0;

	update();
	emit changed();
}

void TF1DCanvas::resetTF()
{
	m_pSelectedKey = 0;
	m_keyColletion->reset();
	emit resetTransferFunction();
	update();
}

void TF1DCanvas::toggleClipThresholds(bool enabled)
{
	m_bIsClipThresholds = enabled;
	if (m_bIsClipThresholds)
		m_xRange = Vector2f(m_fThresholdL, m_fThresholdU);
	else
		m_xRange = Vector2f(0.f, 1.f);

	update();
}

//--------- protected helper functions ---------//

void TF1DCanvas::changeCurrentColor()
{
	if (!m_pSelectedKey || m_bIsNoColor)
		return;

	QColor oldColor;
	oldColor = Col2QColor( m_pSelectedKey->getColor() );

	QColor newColor = QColorDialog::getColor(oldColor, 0);
	if (newColor.isValid())
		changeCurrentColor(newColor);
}

void TF1DCanvas::insertNewKey(Vector2f& hit)
{
	hit.x = max(hit.x,0.f);
	hit.x = min(hit.x,1.f);
	hit.y = max(hit.y,0.f);
	hit.y = min(hit.y,1.f);

	TFMappingKey* key = new TFMappingKey(hit.x, QColor2Col(Qt::lightGray));

	m_keyColletion->addKey(key);
	TFMappingKey* leftKey = getOtherKey(key, true);
	TFMappingKey* rightKey = getOtherKey(key, false);

	// interpolate color of inserted key from neighbouring keys
	// (weighted by distance)
	// the alpha value is determined by hit.y
	Color4 keyColor;
	if (!leftKey)
		keyColor = rightKey->getColor();
	else if (!rightKey)
		keyColor = leftKey->getColor();
	else {
		float leftSource = leftKey->getIntensity();
		float rightSource = rightKey->getIntensity();
		float distSource = rightSource - leftSource;
		Color4 leftColor = static_cast<Color4>(leftKey->getColor());
		Color4 rightColor = static_cast<Color4>(rightKey->getColor());

		keyColor = static_cast<Color4>(
			leftColor* ( (distSource-(hit.x-leftSource))/distSource  ) +
			rightColor*( (distSource-(rightSource-hit.x))/distSource ) );
	}
	key->setColor(keyColor);
	//overwrite alpha value with clicked position
	key->setAlpha(hit.y);

	m_pSelectedKey = key;
}

TFMappingKey* TF1DCanvas::getOtherKey(TFMappingKey* selectedKey, bool chooseLeftPart)
{
	TFMappingKey* otherKey = 0;
	for (int i=0; i < m_keyColletion->getNumKeys(); ++i) {
		if ((chooseLeftPart && i < m_keyColletion->getNumKeys() - 1 && m_keyColletion->getKey(i + 1) == selectedKey) ||
			( !chooseLeftPart && i > 0 && m_keyColletion->getKey(i - 1) == selectedKey)) {
				otherKey = m_keyColletion->getKey(i);
		}
	}
	return otherKey;
}

int TF1DCanvas::hitLine(const Vector2f& p)
{
	int hit = -1;
	Vector2f sHit = Vector2f(p.x, static_cast<float>(height()) - p.y);
	Vector2f old;
	for (int i=0; i < m_keyColletion->getNumKeys(); ++i) {
		TFMappingKey* key = m_keyColletion->getKey(i);
		Vector2f p = wtos(Vector2f(key->getIntensity(), key->getColor().a / 255.f));
		if (i > 0) {
			Vector2f p1 = Vector2f(old.x + 1.f, old.y);
			Vector2f p2 = Vector2f(p.x - 1.f, p.y);
			float s = (p2.y - p1.y) / (p2.x - p1.x);
			int a = static_cast<int>(p1.y + (sHit.x - p1.x) * s);
			if ((sHit.x >= p1.x+10) && (sHit.x <= p2.x-10) && (abs(static_cast<int>(sHit.y) - a) < 5)) {
				hit = i - 1;
			}

			old = p;
		}
	}

	return hit;
}

void TF1DCanvas::paintKeys(QPainter& paint)
{
	for (int i=0; i<m_keyColletion->getNumKeys(); ++i) {
		TFMappingKey *key = m_keyColletion->getKey(i);
		Vector2f p = wtos(Vector2f(key->getIntensity(), key->getColor().a / 255.0));
		int props;
		props = MARKER_NORMAL;
		if (key == m_pSelectedKey)
			props |= MARKER_SELECTED;
		drawMarker(paint, key->getColor(), p, props);
	}
}

void TF1DCanvas::drawMarker(QPainter& paint, const Color4& color, const Vector2f& p, int props)
{
	if (m_bIsNoColor)
		paint.setBrush(Qt::transparent);
	else
		paint.setBrush(Col2QColor(color));

	QPen pen(QBrush(Qt::darkGray), Qt::SolidLine);
	if (props & MARKER_SELECTED)
		pen.setWidth(3);
	paint.setPen(pen);

	paint.drawEllipse(QRectF(p.x - m_iPointSize/2, p.y - m_iPointSize/2, m_iPointSize, m_iPointSize));
}

QColor TF1DCanvas::Col2QColor(const Color4& color)
{
	return QColor(color.r, color.g, color.b); // ignore alpha
}

Color4 TF1DCanvas::QColor2Col(const QColor& color)
{
	return Color4(color.red(), color.green(), color.blue(), 255); // ignore alpha
}

Vector2f TF1DCanvas::wtos(Vector2f p)
{
	float sx = (p.x - m_xRange[0]) / (m_xRange[1] - m_xRange[0]) * (static_cast<float>(width()) \
		- 2 * m_iPadding - 1.5 * m_iArrowLength) + m_iPadding;
	float sy = (p.y - m_yRange[0]) / (m_yRange[1] - m_yRange[0]) * (static_cast<float>(height()) \
		- m_iPadding - 1.5 * m_iArrowLength - m_iBottomSpace) + m_iBottomSpace;
	return Vector2f(sx, sy);
}

Vector2f TF1DCanvas::stow(Vector2f p)
{
	float wx = (p.x - m_iPadding) / (static_cast<float>(width())  - \
		2 * m_iPadding - 1.5 * m_iArrowLength) * (m_xRange[1] - m_xRange[0]) + m_xRange[0];
	float wy = (p.y - m_iPadding - m_iBottomSpace) / (static_cast<float>(height()) - \
		m_iPadding - 1.5 * m_iArrowLength - m_iBottomSpace) * (m_yRange[1] - m_yRange[0]) + m_yRange[0];
	return Vector2f(wx, wy);
}

//--------- additional functions ---------//

QSize TF1DCanvas::minimumSizeHint () const
{
	return QSize(300,100);
}

QSize TF1DCanvas::sizeHint () const
{
	return QSize(300, 100);
}

QSizePolicy TF1DCanvas::sizePolicy () const
{
	return QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void TF1DCanvas::setThreshold(float l, float u)
{
	m_fThresholdL = l;
	m_fThresholdU = u;
	if (m_bIsClipThresholds)
		m_xRange = Vector2f(m_fThresholdL, m_fThresholdU);

	update();
}

void TF1DCanvas::hideCoordinates()
{
	QToolTip::hideText();
}

void TF1DCanvas::updateCoordinates(QPoint pos, Vector2f values)
{
	std::ostringstream os;
	os.precision(2);
	os.setf(std::ios::fixed, std::ios::floatfield);
	os << values.x*m_iMaximumIntensity << " / " << values.y*255.f;
	QToolTip::showText(mapToGlobal(pos), QString(os.str().c_str()));
}

void TF1DCanvas::setTransFunc(TFMappingKeyCollection* tf)
{
	m_keyColletion = tf;
	m_pSelectedKey = 0;
	repaint();
}

void TF1DCanvas::setXAxisText(const std::string& text)
{
	m_xAxisText = QString(text.c_str());
}

void TF1DCanvas::setYAxisText(const std::string& text)
{
	m_yAxisText = QString(text.c_str());
}

void TF1DCanvas::setupTransfunc()
{
	m_keys = m_keyColletion->getKeyCollection();
	unsigned char *transfuncArray = m_pTransfunc->getTFData();
	for (int x = 0; x < 256; ++x)
	{
		Color4 temp = getMappingKeyValue(static_cast<float>(x) / 255.f);
		transfuncArray[x*4] = temp.r;
		transfuncArray[x*4+1] = temp.g;
		transfuncArray[x*4+2] = temp.b;
		transfuncArray[x*4+3] = temp.a;
	}
	emit sendTF(transfuncArray);
}

Color4 TF1DCanvas::getMappingKeyValue( float value ) const
{
	if (m_keys.empty())
		return Color4(0, 0, 0, 0);

	// Restrict value to [0,1]
	value = (value < 0.f) ? 0.f : value;
	value = (value > 1.f) ? 1.f : value;

	// iterate through all keys until we get to the correct position
	std::vector<TFMappingKey*>::const_iterator keyIterator = m_keys.begin();

	while ((keyIterator != m_keys.end()) && (value > (*keyIterator)->getIntensity()))
		keyIterator++;

	if (keyIterator == m_keys.begin())
		return m_keys[0]->getColor();
	else if (keyIterator == m_keys.end())
		return (*(keyIterator-1))->getColor();
	else{
		// calculate the value weighted by the destination to the next left and right key
		TFMappingKey* leftKey = *(keyIterator-1);
		TFMappingKey* rightKey = *keyIterator;
		float fraction = (value - leftKey->getIntensity()) / (rightKey->getIntensity() - leftKey->getIntensity());
		Color4 leftDest = leftKey->getColor();
		Color4 rightDest = rightKey->getColor();
		Color4 result = leftDest;
		result.r += static_cast<uint>((rightDest.r - leftDest.r) * fraction);
		result.g += static_cast<uint>((rightDest.g - leftDest.g) * fraction);
		result.b += static_cast<uint>((rightDest.b - leftDest.b) * fraction);
		result.a += static_cast<uint>((rightDest.a - leftDest.a) * fraction);
		return result;
	}
}

void TF1DCanvas::setHistogram( IntensityHistogram* histogram )
{
	m_pHistogramPainter->setHistogram(histogram);
	update();
}

void TF1DCanvas::load()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open TF"), "G:\\User_Easterman\\Desktop\\TF",
		tr("1DTF File (*.1dtf)"));
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
		return;
	unsigned char *transfuncArray = m_pTransfunc->getTFData();
	file.read((char*)transfuncArray, m_pTransfunc->INTENSITY * m_pTransfunc->ELEMENT);
// 	for (int x = 0; x < 256; ++x)
// 	{
// 		Color4 temp = getMappingKeyValue(static_cast<float>(x) / 255.f);
// 		transfuncArray[x*4] = temp.r;
// 		transfuncArray[x*4+1] = temp.g;
// 		transfuncArray[x*4+2] = temp.b;
// 		transfuncArray[x*4+3] = temp.a;
// 	}
	emit sendTF(transfuncArray);
}

bool TF1DCanvas::save()
{
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Sematic"), "datasets/",
		tr("img (*.png)"));
	if(fileName.isEmpty())
	{
		cout<<"write error"<<endl;
		return false;
	}
	uchar *transfuncArray = m_pTransfunc->getTFData();
	uchar *transRGBA2ARGB = new uchar[256 * 4];
	for (int i = 0 ; i < 256; i++)
	{
		transRGBA2ARGB[i * 4] = transfuncArray[i * 4 + 2];
		transRGBA2ARGB[i * 4 + 1] = transfuncArray[i * 4 + 1];
		transRGBA2ARGB[i * 4 + 2] = transfuncArray[i * 4];
		transRGBA2ARGB[i * 4 + 3] = transfuncArray[i * 4 + 3];

	}
	
	QImage TFImage = QImage(transRGBA2ARGB, 256, 1, QImage::Format_ARGB32);
	TFImage.save(fileName, "PNG");
	delete []transRGBA2ARGB;
	QFile datafile("datasets/sematics_head.txt");
	if (!datafile.open(QFile::WriteOnly|QIODevice::Append))
	{
		cout<<"open error!"<<endl;
	}
	QTextStream data(&datafile);
	QString semantic = fileName.section(QRegExp("[/.]"), 8, 8).trimmed();
	data<<fileName.section(QRegExp("[/.]"), 8, 8).trimmed()<<"\r\n";
	datafile.close();
	return true;
}


void TF1DCanvas::getKeys(unsigned char* TF)
{
	//unsigned char *transfuncArray = m_pTransfunc->getTFData();
	//memcpy(transfuncArray, TF, m_pTransfunc->INTENSITY * m_pTransfunc->ELEMENT);
	//m_keyColletion->reset();
	//TFMappingKey *key = m_keyColletion->getKey(0);
	////key = m_keyColletion->getKey(1);
	//Color4 temp;
	//temp.r = transfuncArray[0];
	//temp.g = transfuncArray[1];
	//temp.b = transfuncArray[2];
	//temp.a = transfuncArray[3];
	//key->setColor(temp);
	//key->setIntensity(0);
	//key = m_keyColletion->getKey(1);
	//m_keyColletion->removeKey(key);
	//for (int x = 5; x < 256; x += 25)
	//{
	//	temp.r = transfuncArray[4 * x];
	//	temp.g = transfuncArray[4 * x + 1];
	//	temp.b = transfuncArray[4 * x + 2];
	//	temp.a = transfuncArray[4 * x + 3];
	//	//cout<<(int)temp.r<<" "<<(int)temp.g<<" "<<(int)temp.b<<" "<<(int)temp.a<<endl;
	//	key = new TFMappingKey();
	//	key->setColor(temp);
	//	key->setIntensity(static_cast<float>(x)/255.f);
	//	m_keyColletion->addKey(key);
	//}

	m_keyColletion->reset();
	TFMappingKey *key = m_keyColletion->getKey(1);
	m_keyColletion->removeKey(key);
	key = m_keyColletion->getKey(0);
	m_keyColletion->removeKey(key);
	int i, index=0;
	Color4 temp;
	float intensity;
	for (i = 0; i < 50;)
	{
		if(index >= 256)
			break;
		temp.r = TF[4 * index];
		temp.g = TF[4 * index + 1];
		temp.b = TF[4 * index + 2];
		temp.a = TF[4 * index + 3];
		index += 5;
		if (temp.a)
		{
			if (!m_keyColletion->getNumKeys())
			{
				Color4 firstkey = temp;
				firstkey.a = 0;
				key = new TFMappingKey();
				key->setColor(firstkey);
				intensity = static_cast<float>(index-10)/255.f;
				if(intensity < 0)
					intensity = 0;
				else if(intensity > 1)
					intensity = 1;
				key->setIntensity(intensity);
				m_keyColletion->addKey(key);
				i++;

			}
			
			key = new TFMappingKey();
			key->setColor(temp);
			intensity = static_cast<float>(index-5)/255.f;
			if(intensity < 0)
				intensity = 0;
			else if(intensity > 1)
				intensity = 1;
			key->setIntensity(intensity);
			m_keyColletion->addKey(key);
			i++;
		}
		else
			continue;
		
	}
	key = m_keyColletion->getKey(i-1);
	intensity = key->getIntensity();
	temp = key->getColor();
	temp.a = 0;
	key = new TFMappingKey();
	key->setColor(temp);
	intensity += 5.0/255.f;
	if (intensity < 0)
		intensity = 0;
	else if(intensity > 1)
		intensity = 1;
	key->setIntensity(intensity);
	m_keyColletion->addKey(key);

	update();
	
}