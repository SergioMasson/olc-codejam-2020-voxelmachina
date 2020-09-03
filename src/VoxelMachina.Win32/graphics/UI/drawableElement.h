#pragma once

namespace graphics
{
	namespace UI
	{
		enum class ParentAnchorType
		{
			TopLeft,
			TopRight,
			BottomLeft,
			BottomRight,
			Center
		};

		class DrawableElement
		{
		public:
			DrawableElement(DrawableElement* parent, float x, float y, float width, float height, ParentAnchorType anchorType = ParentAnchorType::TopLeft) :
				m_parent{ parent },
				m_anchorType{ anchorType },
				m_X{ x },
				m_Y{ y },
				m_Width{ width },
				m_Height{ height }{};

			virtual void Draw(void) = 0;
			void GetScreenPoition(float& x, float& y);
			void SetLocalPosition(float x, float y);
			void SetAnchorType(ParentAnchorType type) { m_anchorType = type; }

		protected:

			float m_X;
			float m_Y;
			float m_Width;
			float m_Height;

			DrawableElement* m_parent = nullptr;
			ParentAnchorType m_anchorType;
		};
	}
}