#pragma once

#include "drawableElement.h"

namespace graphics
{
	namespace UI
	{
		class GuiSprite : public DrawableElement
		{
		public:
			GuiSprite(DrawableElement* parent, float x, float y, float width, float height);

			void LoadBitmapFromFile(std::wstring filePath, bool setNativeSize = false);
			virtual void Draw(void) override;

		private:
			Microsoft::WRL::ComPtr<ID2D1Bitmap> m_bitmap;
		};
	}
}