#pragma once

#include "drawableElement.h"

namespace graphics
{
	namespace UI
	{
		class GuiText : public DrawableElement
		{
		public:
			GuiText(DrawableElement* parent, float x, float y, float width, float height, float fontSize);
			virtual void Draw(void) override;

			void SetText(std::wstring text) { m_text = text; }
			std::wstring GetText() const { return m_text; };

			void SetColor(D2D1::ColorF color);
			D2D1::ColorF GetColor() const { return m_color; };

		private:
			Microsoft::WRL::ComPtr<IDWriteTextFormat> m_writer = nullptr;
			Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush = nullptr;
			std::wstring m_text;
			D2D1::ColorF m_color;
		};
	}
}