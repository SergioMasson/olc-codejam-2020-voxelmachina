#include "pch.h"
#include "guiPanel.h"
#include "../coreGraphics.h"

graphics::UI::GuiPanel::GuiPanel(DrawableElement* parent, float x, float y, float width, float height) : DrawableElement{ parent, x, y, width, height }, m_color{ D2D1::ColorF::Black }
{
	SetColor(D2D1::ColorF::Black);
}

void graphics::UI::GuiPanel::Draw(void)
{
	float x, y;
	GetScreenPoition(x, y);
	auto drawRect = D2D1::RectF(x, y, m_Width + x, m_Height + y);
	graphics::g_d2dDeviceContext->FillRectangle(drawRect, m_brush.Get());
}

void graphics::UI::GuiPanel::SetColor(Color color)
{
	m_brush.Reset();
	m_color = color;
	ASSERT_SUCCEEDED(graphics::g_d2dDeviceContext->CreateSolidColorBrush(color, m_brush.GetAddressOf()));
}