#include "pch.h"
#include "drawableElement.h"
#include "../coreGraphics.h"

void graphics::UI::DrawableElement::GetScreenPoition(float& x, float& y)
{
	float parentX = 0;
	float parentY = 0;
	float parentWidth;
	float parentHeight;

	if (m_parent == nullptr)
	{
		parentWidth = static_cast<float>(graphics::g_windowWidth);
		parentHeight = static_cast<float>(graphics::g_windowHeight);
	}
	else
	{
		m_parent->GetScreenPoition(parentX, parentY);
		parentWidth = m_parent->m_Width;
		parentHeight = m_parent->m_Height;
	}

	switch (m_anchorType)
	{
	case graphics::UI::ParentAnchorType::TopLeft:
		x = parentX + m_X;
		y = parentY + m_Y;
		break;
	case graphics::UI::ParentAnchorType::TopRight:
		x = parentX + (parentWidth)+m_X;
		y = parentY + m_Y;
		break;
	case graphics::UI::ParentAnchorType::BottomLeft:
		x = parentX + m_X;
		y = parentY + (parentHeight)+m_Y;
		break;
	case graphics::UI::ParentAnchorType::BottomRight:
		x = parentX + (parentWidth)+m_X;
		y = parentY + (parentHeight)+m_Y;
		break;
	case graphics::UI::ParentAnchorType::Center:
		x = parentX + m_X;
		y = parentY + m_Y;
		break;
	default:
		break;
	}
}

void graphics::UI::DrawableElement::SetLocalPosition(float x, float y)
{
	m_X = x;
	m_Y = y;
}