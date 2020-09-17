#include "pch.h"
#include "../../colors.h"
#include "guiText.h"
#include "../coreGraphics.h"

static const WCHAR msc_fontName[] = L"Verdana";

graphics::UI::GuiText::GuiText(DrawableElement* parent, float x, float y, float width, float height, float fontSize) : DrawableElement{ parent, x, y, width, height }, m_color{ D2D1::ColorF::Black }
{
	Microsoft::WRL::ComPtr<IDWriteFactory> m_pDWriteFactory;

	auto hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>(m_pDWriteFactory.GetAddressOf()));

	ASSERT_SUCCEEDED(hr);

	// Create a DirectWrite text format object.
	hr = m_pDWriteFactory->CreateTextFormat(
		msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		L"", //locale
		m_writer.GetAddressOf()
	);

	ASSERT_SUCCEEDED(hr);

	SetTextAlignment(TextAlignment::Justified);
	m_writer->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	SetColor(D2D1::ColorF::Black);
}

void graphics::UI::GuiText::Draw(void)
{
	float x, y;
	GetScreenPoition(x, y);
	auto drawRect = D2D1::RectF(x, y, m_Width + x, m_Height + y);
	graphics::g_d2dDeviceContext->DrawText(m_text.c_str(), static_cast<int>(m_text.size()), m_writer.Get(), drawRect, m_brush.Get());
}

void graphics::UI::GuiText::SetColor(Color color)
{
	m_brush.Reset();
	ASSERT_SUCCEEDED(graphics::g_d2dDeviceContext->CreateSolidColorBrush(color, m_brush.GetAddressOf()));
	m_color = color;
}

void graphics::UI::GuiText::SetFontSize(float fontSize)
{
	m_writer.Reset();

	Microsoft::WRL::ComPtr<IDWriteFactory> m_pDWriteFactory;

	auto hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>(m_pDWriteFactory.GetAddressOf()));
	ASSERT_SUCCEEDED(hr);

	// Create a DirectWrite text format object.
	hr = m_pDWriteFactory->CreateTextFormat(
		msc_fontName,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		L"", //locale
		m_writer.GetAddressOf()
	);

	m_fontSize = fontSize;
}

void graphics::UI::GuiText::SetTextAlignment(TextAlignment alignment)
{
	m_writer->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(alignment));
	m_alignment = alignment;
}