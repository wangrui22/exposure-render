
#include "TransferFunction.h"

QTransferFunction gTransferFunction;

// Compare two transfer function nodes by position
bool CompareNodes(QNode* pNodeA, QNode* pNodeB)
{
	return pNodeA->GetPosition() < pNodeB->GetPosition();
}

QNode::QNode(const float& Position, const float& Opacity, const QColor& Color, const bool& Deletable) :
	QObject(),
	m_Position(Position),
	m_Opacity(Opacity),
	m_Color(Color),
	m_Deletable(Deletable),
	m_AllowMoveH(Deletable),
	m_AllowMoveV(true),
	m_MinX(0.0f),
	m_MaxX(255.0f),
	m_MinY(0.0f),
	m_MaxY(1.0f)
{
}

float QNode::GetX(void) const
{
	return GetPosition();
}

void QNode::SetX(const float& X)
{
	SetPosition(X);
}

float QNode::GetY(void) const
{
	return GetOpacity();
}

void QNode::SetY(const float& Y)
{
	SetOpacity(Y);
}

float QNode::GetNormalizedX(void) const 
{
	return (GetPosition() - gTransferFunction.m_RangeMin) / gTransferFunction.m_Range;
}

void QNode::SetNormalizedX(const float& NormalizedX)
{

	SetPosition(gTransferFunction.m_RangeMin + (gTransferFunction.m_Range * NormalizedX));
}

float QNode::GetNormalizedY(void) const 
{
	return GetOpacity();
}

void QNode::SetNormalizedY(const float& NormalizedY)
{
	SetOpacity(NormalizedY);
}

float QNode::GetPosition(void) const
{
	return m_Position;
}

void QNode::SetPosition(const float& Position)
{
	m_Position = qMin(m_MaxX, qMax(Position, m_MinX));
	
	emit NodeChanged(this);
	emit PositionChanged(this);
}

float QNode::GetOpacity(void) const
{
	return m_Opacity;
}

void QNode::SetOpacity(const float& Opacity)
{
	m_Opacity = qMin(m_MaxY, qMax(Opacity, m_MinY));
	m_Opacity = Opacity;

	emit NodeChanged(this);
	emit OpacityChanged(this);
}

QColor QNode::GetColor(void) const
{
	return m_Color;
}

void QNode::SetColor(const QColor& Color)
{
	m_Color = Color;
	
	emit ColorChanged(this);
}

float QNode::GetMinX(void) const
{
	return m_MinX;
}

void QNode::SetMinX(const float& MinX)
{
	m_MinX = MinX;

	emit RangeChanged(this);
}

float QNode::GetMaxX(void) const
{
	return m_MaxX;
}

void QNode::SetMaxX(const float& MaxX)
{
	m_MaxX = MaxX;

	emit RangeChanged(this);
}

float QNode::GetMinY(void) const
{
	return m_MinY;
}

void QNode::SetMinY(const float& MinY)
{
	m_MinY = MinY;

	emit RangeChanged(this);
}

float QNode::GetMaxY(void) const
{
	return m_MaxY;
}

void QNode::SetMaxY(const float& MaxY)
{
	m_MaxY = MaxY;

	emit RangeChanged(this);
}

bool QNode::InRange(const QPointF& Point)
{
	return Point.x() >= m_MinX && Point.x() <= m_MaxX && Point.y() >= m_MinY && Point.y() <= m_MaxY;
}

QPointF QNode::RestrictToRange(const QPointF& Point)
{
	QPointF NewPoint;

	NewPoint.setX(qMin(m_MaxX, qMax((float)Point.x(), m_MinX)));
	NewPoint.setY(qMin(m_MaxY, qMax((float)Point.y(), m_MinY)));

	return NewPoint;
}

QTransferFunction::QTransferFunction(QObject* pParent) :
	QObject(pParent),
	m_Nodes(),
	m_RangeMin(0.0f),
	m_RangeMax(255.0f),
	m_Range(m_RangeMax - m_RangeMin),
	m_pSelectedNode(NULL)
{
}

void QTransferFunction::SetSelectedNode(QNode* pSelectedNode)
{
	m_pSelectedNode = pSelectedNode;
	emit SelectionChanged(m_pSelectedNode);
}

void QTransferFunction::SetSelectedNode(const int& Index)
{
	if (m_Nodes.size() <= 0)
		return;

	// Compute new index
	const int NewIndex = qMin(m_Nodes.size(), qMax(0, Index));

	// Set selected node
	m_pSelectedNode = m_Nodes[NewIndex];

	// Notify others that our selection has changed
	emit SelectionChanged(m_pSelectedNode);
}

void QTransferFunction::OnNodeChanged(QNode* pNode)
{
	UpdateNodeRanges();

	emit FunctionChanged();
}

void QTransferFunction::SelectPreviousNode(void)
{
	if (!m_pSelectedNode)
		return;

	int Index = GetNodeIndex(m_pSelectedNode);

	if (Index < 0)
		return;

	// Compute new index
	const int NewIndex = qMin(m_Nodes.size() - 1, qMax(0, Index - 1));

	// Set selected node
	SetSelectedNode(m_Nodes[NewIndex]);
}

void QTransferFunction::SelectNextNode(void)
{
	if (!m_pSelectedNode)
		return;

	int Index = GetNodeIndex(m_pSelectedNode);

	if (Index < 0)
		return;

	// Compute new index
	const int NewIndex = qMin(m_Nodes.size() - 1, qMax(0, Index + 1));

	// Set selected node
	SetSelectedNode(m_Nodes[NewIndex]);
}

int	QTransferFunction::GetNodeIndex(QNode* pNode)
{
	if (pNode == NULL)
		return -1;

	return m_Nodes.indexOf(pNode);
}

void QTransferFunction::AddNode(QNode* pNode)
{
	m_Nodes.append(pNode);

	// Sort the transfer function nodes
	qSort(gTransferFunction.m_Nodes.begin(), gTransferFunction.m_Nodes.end(), CompareNodes);

	// Update node's range
	UpdateNodeRanges();

	// Emit
	emit NodeAdd(m_Nodes.back());
	emit FunctionChanged();

	// Notify us when the node changes
	connect(pNode, SIGNAL(NodeChanged(QNode*)), this, SLOT(OnNodeChanged(QNode*)));
}

void QTransferFunction::RemoveNode(QNode* pNode)
{
	// Let others know that we are about to remove a node
	emit NodeRemove(pNode);

	// Remove the connection
	disconnect(pNode, SIGNAL(NodeChanged(QNode*)), this, SLOT(OnNodeChanged(QNode*)));

	// Remove from list and memory
	m_Nodes.remove(m_Nodes.indexOf(pNode));
	delete pNode;

	// Update node's range
	UpdateNodeRanges();

	// Let others know that we remove a node, and that the transfer function has changed
	emit NodeRemoved(pNode);

	// Emit
	emit FunctionChanged();
}

void QTransferFunction::UpdateNodeRanges(void)
{
	// Compute the node ranges
	for (int i = 0; i < m_Nodes.size(); i++)
	{
		QNode* pNode = gTransferFunction.m_Nodes[i];

		if (pNode == gTransferFunction.m_Nodes.front())
		{
			pNode->SetMinX(0.0f);
			pNode->SetMaxX(0.0f);
		}
		else if (pNode == gTransferFunction.m_Nodes.back())
		{
			pNode->SetMinX(gTransferFunction.m_RangeMax);
			pNode->SetMaxX(gTransferFunction.m_RangeMax);
		}
		else
		{
			QNode* pNodeLeft	= gTransferFunction.m_Nodes[i - 1];
			QNode* pNodeRight	= gTransferFunction.m_Nodes[i + 1];

			pNode->SetMinX(pNodeLeft->GetPosition());
			pNode->SetMaxX(pNodeRight->GetPosition());
		}
	}
}

void QTransferFunction::SetHistogram(const int* pBins, const int& NoBins)
{
	m_Histogram.m_Bins.clear();

	for (int i = 0; i < NoBins; i++)
	{
		if (pBins[i] > m_Histogram.m_Max)
			m_Histogram.m_Max = pBins[i];
	}

	for (int i = 0; i < NoBins; i++)
		m_Histogram.m_Bins.append(pBins[i]);
}