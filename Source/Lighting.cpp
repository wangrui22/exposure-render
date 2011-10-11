
// Precompiled headers
#include "Stable.h"

#include "Lighting.h"

QLighting gLighting;

QLighting::QLighting(QObject* pParent /*= NULL*/) :
	QPresetXML(pParent),
	m_Lights(),
	m_pSelectedLight(NULL),
	m_Background()
{
}

QLighting::~QLighting(void)
{
}

QLighting::QLighting(const QLighting& Other)
{
	*this = Other;
}

QLighting& QLighting::operator=(const QLighting& Other)
{
	// Clear light selection, do not remove this line!
	SetSelectedLight((QLight*)NULL);

	blockSignals(true);

	QPresetXML::operator=(Other);

	QObject::disconnect(this, SLOT(OnLightPropertiesChanged(QLight*)));

	m_Lights		= Other.m_Lights;
	
	blockSignals(false);

	m_Background = Other.m_Background;

	emit Changed();

	for (int i = 0; i < m_Lights.size(); i++)
	{
		QObject::connect(&m_Lights[i], SIGNAL(LightPropertiesChanged(QLight*)), this, SLOT(OnLightPropertiesChanged(QLight*)));
	}

	return *this;
}

void QLighting::OnLightPropertiesChanged(QLight* pLight)
{
	emit Changed();
}

void QLighting::OnBackgroundChanged(void)
{
	emit Changed();
}

void QLighting::AddLight(QLight& Light)
{
	// Add to list
	m_Lights.append(Light);

	// Select
	SetSelectedLight(&m_Lights.back());

	// Connect
	connect(&m_Lights.back(), SIGNAL(LightPropertiesChanged(QLight*)), this, SLOT(OnLightPropertiesChanged(QLight*)));

	Log("'" + Light.GetName() + "' added to the scene", "light-bulb");

	// Let others know the lighting has changed
	emit Changed();
}

void QLighting::RemoveLight(QLight* pLight)
{
	if (!pLight)
		return;

	Log("'" + pLight->GetName() + "' removed from the scene", "light-bulb");

	// Remove from light list
	m_Lights.remove(*pLight);

	m_pSelectedLight = NULL;

	// Deselect
	SetSelectedLight(NULL);

	// Let others know the lighting has changed
	emit Changed();
}

void QLighting::RemoveLight(const int& Index)
{
	if (Index < 0 || Index >= m_Lights.size())
		return;

	RemoveLight(&m_Lights[Index]);
}

QBackground& QLighting::Background(void)
{
	return m_Background;
}

QLightList& QLighting::GetLights(void)
{
	return m_Lights;
}

void QLighting::SetSelectedLight(QLight* pSelectedLight)
{
	m_pSelectedLight = pSelectedLight;
	emit LightSelectionChanged(pSelectedLight);
}

void QLighting::SetSelectedLight(const int& Index)
{
	if (m_Lights.size() <= 0)
	{
		SetSelectedLight((QLight*)NULL);
	}
	else
	{
		// Compute new index
		const int NewIndex = qMin(m_Lights.size() - 1, qMax(0, Index));

		if (GetSelectedLight() && m_Lights.indexOf(*GetSelectedLight()) == NewIndex)
			return;

		// Set selected node
		SetSelectedLight(&m_Lights[NewIndex]);
	}
}

QLight* QLighting::GetSelectedLight(void)
{
	return m_pSelectedLight;
}

void QLighting::SelectPreviousLight(void)
{
	if (!m_pSelectedLight)
		return;

	int Index = m_Lights.indexOf(*GetSelectedLight());

	if (Index < 0)
		return;

	// Compute new index
	const int NewIndex = qMin(m_Lights.size() - 1, qMax(0, Index - 1));

	// Set selected node
	SetSelectedLight(&m_Lights[NewIndex]);
}

void QLighting::SelectNextLight(void)
{
	if (!m_pSelectedLight)
		return;

	int Index = m_Lights.indexOf(*GetSelectedLight());

	if (Index < 0)
		return;

	// Compute new index
	const int NewIndex = qMin(m_Lights.size() - 1, qMax(0, Index + 1));

	// Set selected node
	SetSelectedLight(&m_Lights[NewIndex]);
}

void QLighting::CopyLight(QLight* pLight)
{
	if (!pLight)
		return;

	QLight LightCopy = *pLight;

	// Rename
	LightCopy.SetName("Copy of " + pLight->GetName());

	// Add
	AddLight(LightCopy);

	// Let others know the lighting has changed
	emit Changed();
}

void QLighting::CopySelectedLight(void)
{
	CopyLight(m_pSelectedLight);
}

void QLighting::RenameLight(const int& Index, const QString& Name)
{
	if (Index < 0 || Index >= m_Lights.size() || Name.isEmpty())
		return;

	Log("'" + m_Lights[Index].GetName() + " renamed to '" + Name + "'", "light-bulb");

	m_Lights[Index].SetName(Name);

	// Let others know the lighting has changed
	emit Changed();
}

void QLighting::ReadXML(QDomElement& Parent)
{
	SetSelectedLight(NULL);

	QPresetXML::ReadXML(Parent);

	QDomElement Lights = Parent.firstChild().toElement();

	// Read child nodes
	for (QDomNode DomNode = Lights.firstChild(); !DomNode.isNull(); DomNode = DomNode.nextSibling())
	{
		// Create new light preset
		QLight LightPreset(this);

		m_Lights.append(LightPreset);

		// Load preset into it
		m_Lights.back().ReadXML(DomNode.toElement());
	}

	QDomElement Background = Parent.firstChildElement("Background").toElement();
	m_Background.ReadXML(Background);

	SetSelectedLight(0);
}

QDomElement QLighting::WriteXML(QDomDocument& DOM, QDomElement& Parent)
{
	// Preset
	QDomElement Preset = DOM.createElement("Preset");
	Parent.appendChild(Preset);

	QPresetXML::WriteXML(DOM, Preset);

	QDomElement Lights = DOM.createElement("Lights");
	Preset.appendChild(Lights);

	for (int i = 0; i < m_Lights.size(); i++)
		m_Lights[i].WriteXML(DOM, Lights);

	m_Background.WriteXML(DOM, Preset);

	return Preset;
}

QLighting QLighting::Default(void)
{
	QLighting DefaultLighting;

	DefaultLighting.SetName("Default");

	QLight Key;
	Key.SetTheta(180.0f);
	Key.SetPhi(45.0f);
	Key.SetName("Key");
	Key.SetColor(QColor(255, 228, 165));
	Key.SetWidth(0.7f);
	Key.SetHeight(0.7f);
	Key.SetIntensity(5000.0f);
	Key.SetDistance(1.25f);

	DefaultLighting.AddLight(Key);

	QLight Rim;
	Rim.SetTheta(90.0f);
	Rim.SetPhi(25.0f);
	Rim.SetName("Rim");
	Rim.SetColor(QColor(155, 155, 205));
	Rim.SetWidth(1.7f);
	Rim.SetHeight(1.7f);
	Rim.SetIntensity(3000.0f);
	Rim.SetDistance(2.25f);

	DefaultLighting.AddLight(Rim);

	DefaultLighting.Background().SetTopColor(QColor(185, 213, 255));
	DefaultLighting.Background().SetMiddleColor(QColor(185, 213, 255));
	DefaultLighting.Background().SetBottomColor(QColor(185, 213, 255));

	DefaultLighting.Background().SetIntensity(300.0f);

	return DefaultLighting;
}