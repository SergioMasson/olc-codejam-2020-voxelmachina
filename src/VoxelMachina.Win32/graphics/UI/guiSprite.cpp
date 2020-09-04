#include "pch.h"
#include "guiSprite.h"
#include "../coreGraphics.h"

using namespace Microsoft::WRL;

graphics::UI::GuiSprite::GuiSprite(DrawableElement* parent, float x, float y, float width, float height) : DrawableElement{ parent, x, y, width, height }
{
}

void graphics::UI::GuiSprite::LoadBitmapFromFile(std::wstring filePath, bool setNativeSize)
{
	ComPtr<IWICImagingFactory> wicFactory = nullptr;

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)wicFactory.GetAddressOf());

	ASSERT_SUCCEEDED(hr, "Fail to create IWICImagingFactory instance");

	ComPtr<IWICBitmapDecoder> wicDecoder;

	hr = wicFactory->CreateDecoderFromFilename(filePath.c_str(),
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		wicDecoder.GetAddressOf()
	);

	ASSERT_SUCCEEDED(hr, "Fail to create IWICBitmapDecoder");

	ComPtr<IWICBitmapFrameDecode> pSource = NULL;

	ASSERT_SUCCEEDED(wicDecoder->GetFrame(0, pSource.GetAddressOf()), "Fail to Get frame 0");

	ComPtr<IWICFormatConverter> pConverter = NULL;

	// Convert the image format to 32bppPBGRA
	// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
	ASSERT_SUCCEEDED(wicFactory->CreateFormatConverter(pConverter.GetAddressOf()), "Fail to create IWICFormatConverter");

	hr = pConverter->Initialize(
		pSource.Get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.f,
		WICBitmapPaletteTypeMedianCut
	);

	ASSERT_SUCCEEDED(hr, "Fail to initialize IWICFormatConverter");

	// Create a Direct2D bitmap from the WIC bitmap.
	hr = graphics::g_d2dDeviceContext->CreateBitmapFromWicBitmap(pConverter.Get(), NULL, m_bitmap.GetAddressOf());

	ASSERT_SUCCEEDED(hr, "Fail to create bitmap from wicbitmap");

	if (setNativeSize)
	{
		auto size = m_bitmap->GetSize();
		m_Width = size.width;
		m_Height = size.height;
	}
}

void graphics::UI::GuiSprite::Draw(void)
{
	float x, y;
	GetScreenPoition(x, y);
	auto drawRect = D2D1::RectF(x, y, x + m_Width, y + m_Height);
	D2D1_RECT_F source{ 0, 0, m_bitmap->GetSize().width,  m_bitmap->GetSize().height };
	graphics::g_d2dDeviceContext->DrawBitmap(m_bitmap.Get(), drawRect, 1, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, source);
}