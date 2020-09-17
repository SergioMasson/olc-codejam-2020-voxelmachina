#pragma once

#include "drawableElement.h"
#include "../../colors.h"

enum class TextAlignment
{
	Leading,
	Trailing,
	Center,
	Justified
};

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

			void SetColor(Color color);
			Color GetColor() const { return m_color; };

			void SetFontSize(float fontSize);
			float GetFontSize() const { return m_fontSize; }

			void SetTextAlignment(TextAlignment alignment);
			TextAlignment GetTextAlignment() const { return m_alignment; }

		private:
			Microsoft::WRL::ComPtr<IDWriteTextFormat> m_writer = nullptr;
			Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush = nullptr;
			std::wstring m_text;
			Color m_color;
			TextAlignment m_alignment;
			float m_fontSize;
		};
	}
}