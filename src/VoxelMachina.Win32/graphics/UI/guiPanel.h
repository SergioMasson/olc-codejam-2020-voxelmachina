#pragma once

#include "drawableElement.h"

namespace graphics
{
	namespace UI
	{
		class GuiPanel : public DrawableElement
		{
		public:
			GuiPanel(DrawableElement* parent, float x, float y, float width, float height);
			virtual void Draw(void) override;

			void SetColor(D2D1::ColorF color);
			D2D1::ColorF GetColor() const { return m_color; };

		private:
			Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush = nullptr;
			D2D1::ColorF m_color;
		};
	}
}