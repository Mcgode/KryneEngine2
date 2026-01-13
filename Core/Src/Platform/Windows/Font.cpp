/**
 * @file
 * @author Max Godefroy
 * @date 13/01/2026.
 */

#include "KryneEngine/Core/Platform/Platform.hpp"

#include <dwrite.h>
#include <wrl.h>
#include <string>
#include <d2d1.h>

namespace KryneEngine::Platform
{
    bool RetrieveSystemDefaultGlyph(
        u32 _unicodeCodePoint,
        void* _userData,
        FontGlyphMetricsFunction _fontMetrics,
        FontNewContourFunction _newContour,
        FontNewEdgeFunction _newEdge,
        FontNewConicFunction _newConic,
        FontNewCubicFunction _newCubic,
        FontEndContourFunction _endContour,
        bool _verticalLayout)
    {
        using Microsoft::WRL::ComPtr;

        // Lazily create DWrite factory (simple approach; production code should cache and handle failures better).
        static ComPtr<IDWriteFactory> s_factory;
        if (!s_factory)
        {
            if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(s_factory.GetAddressOf()))))
                return false;
        }

        ComPtr<IDWriteFontCollection> fontCollection;
        if (FAILED(s_factory->GetSystemFontCollection(&fontCollection)))
            return false;

        // Choose a reasonable system default family name. You can change this to user's preference.
        const eastl::wstring familyName = L"Segoe UI";

        UINT32 familyIndex = 0;
        BOOL exists = FALSE;
        if (FAILED(fontCollection->FindFamilyName(familyName.c_str(), &familyIndex, &exists)) || !exists)
        {
            // Fallback to family 0 if "Segoe UI" not present.
            familyIndex = 0;
        }

        ComPtr<IDWriteFontFamily> fontFamily;
        if (FAILED(fontCollection->GetFontFamily(familyIndex, &fontFamily)))
            return false;

        ComPtr<IDWriteFont> font;
        if (FAILED(fontFamily->GetFirstMatchingFont(
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STRETCH_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            &font)))
            return false;

        ComPtr<IDWriteFontFace> fontFace;
        if (FAILED(font->CreateFontFace(&fontFace)))
            return false;

        // Map unicode codepoint to glyph index
        UINT16 glyphIndex = 0;
        UINT32 codePoint = _unicodeCodePoint;
        if (FAILED(fontFace->GetGlyphIndicesA(&codePoint, 1, &glyphIndex)))
            return false;
        if (glyphIndex == 0)
        {
            // glyph index 0 often means missing glyph (depends on font); still attempt if desired.
            // Return false to indicate not found.
            return false;
        }

        // Get design units and metrics to scale coordinates to a convenient unit.
        DWRITE_FONT_METRICS metrics{};
        fontFace->GetMetrics(&metrics);
        // We'll pick an emSize in device-independent units; choose 1000 to make values reasonable.
        const auto designUnitsPerEm = static_cast<FLOAT>(metrics.designUnitsPerEm);

        {
            DWRITE_GLYPH_METRICS dGlyphMetrics{};
            if (SUCCEEDED(fontFace->GetDesignGlyphMetrics(&glyphIndex, 1, &dGlyphMetrics, _verticalLayout)))
            {
                const GlyphMetrics glyphMetrics = {
                    .m_bounds = {
                        dGlyphMetrics.leftSideBearing,
                        dGlyphMetrics.topSideBearing,
                        dGlyphMetrics.rightSideBearing - dGlyphMetrics.leftSideBearing,
                        dGlyphMetrics.topSideBearing - dGlyphMetrics.bottomSideBearing
                    },
                    .m_advance = static_cast<double>(_verticalLayout ? dGlyphMetrics.advanceHeight : dGlyphMetrics.advanceWidth),
                };

                const FontMetrics fontMetrics {
                    .m_ascender = static_cast<double>(metrics.ascent),
                    .m_descender = static_cast<double>(metrics.descent),
                    .m_lineHeight = static_cast<double>(metrics.ascent + metrics.descent + metrics.lineGap),
                    .m_unitPerEm = static_cast<double>(metrics.designUnitsPerEm)
                };

                _fontMetrics(fontMetrics, glyphMetrics, _userData);
            }
            else
            {
                return false;
            }
        }

        // Prepare a small geometry sink that forwards commands to the provided callbacks.
        struct GeometrySink final : IDWriteGeometrySink
        {
            volatile long m_ref;
            void* m_user;
            FontGlyphMetricsFunction m_fontMetrics;
            FontNewContourFunction m_newContour;
            FontNewEdgeFunction m_newEdge;
            FontNewConicFunction m_newConic;
            FontNewCubicFunction m_newCubic;
            FontEndContourFunction m_endContour;

            GeometrySink(
                void* user,
                FontGlyphMetricsFunction fm,
                FontNewContourFunction nc,
                FontNewEdgeFunction ne,
                FontNewConicFunction nco,
                FontNewCubicFunction ncu,
                FontEndContourFunction ec)
                    : m_ref(1)
                    , m_user(user)
                    , m_fontMetrics(fm)
                    , m_newContour(nc)
                    , m_newEdge(ne)
                    , m_newConic(nco)
                    , m_newCubic(ncu)
                    , m_endContour(ec)
            {}

            // IUnknown
            STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
            {
                if (!ppvObject) return E_POINTER;
                if (riid == __uuidof(IUnknown) || riid == __uuidof(IDWriteGeometrySink))
                {
                    *ppvObject = static_cast<IDWriteGeometrySink*>(this);
                    AddRef();
                    return S_OK;
                }
                *ppvObject = nullptr;
                return E_NOINTERFACE;
            }
            STDMETHODIMP_(ULONG) AddRef() override { return InterlockedIncrement(&m_ref); }
            STDMETHODIMP_(ULONG) Release() override
            {
                long v = InterlockedDecrement(&m_ref);
                if (v == 0) delete this;
                return v;
            }

            // IDWriteGeometrySink
            STDMETHODIMP_(void) BeginFigure(D2D1_POINT_2F _startPoint, D2D1_FIGURE_BEGIN _figureBegin) noexcept override
            {
                KE_ASSERT(_figureBegin == D2D1_FIGURE_BEGIN_FILLED);
                m_newContour(
                    { _startPoint.x, _startPoint.y },
                    m_user);
            }

            STDMETHODIMP_(void) AddLines(const D2D1_POINT_2F* points, UINT pointsCount) noexcept override
            {
                for (UINT i = 0; i < pointsCount; ++i)
                {
                    m_newEdge(
                        { points[i].x, points[i].y },
                        m_user);
                }
            }

            STDMETHODIMP_(void) AddBeziers(const D2D1_BEZIER_SEGMENT* beziers, UINT beziersCount) noexcept override
            {
                for (UINT i = 0; i < beziersCount; ++i)
                {
                    const D2D1_BEZIER_SEGMENT& b = beziers[i];
                    m_newCubic(
                        { b.point1.x, b.point1.y },
                        { b.point2.x, b.point2.y },
                        { b.point3.x, b.point3.y },
                        m_user);
                }
            }

            STDMETHODIMP_(void) EndFigure(D2D1_FIGURE_END figureEnd) noexcept override
            {
                KE_ASSERT(figureEnd == D2D1_FIGURE_END_CLOSED);
                m_endContour(m_user);
            }

            STDMETHODIMP_(HRESULT) Close() noexcept override
            {
                return S_OK;
            }

            STDMETHODIMP_(void) SetFillMode(D2D1_FILL_MODE fillMode) noexcept override {}
            STDMETHODIMP_(void) SetSegmentFlags(D2D1_PATH_SEGMENT vertexFlags) noexcept override {}
        };

        // Build glyph arrays for a single glyph.
        UINT16 glyphIndices[1] = { glyphIndex };

        // Create our sink and call GetGlyphRunOutline.
        GeometrySink sinkRaw(
            _userData,
            _fontMetrics,
            _newContour,
            _newEdge,
            _newConic,
            _newCubic,
            _endContour);

        ComPtr<IDWriteGeometrySink> sink;
        sink.Attach(&sinkRaw);

        // Call GetGlyphRunOutline: emSize is design scaled already above.
        HRESULT hr = fontFace->GetGlyphRunOutline(
            designUnitsPerEm,
            glyphIndices,
            nullptr,
            nullptr,
            1,
            _verticalLayout,
            FALSE,
            sink.Get());

        if (FAILED(hr))
        {
            return false;
        }

        return true;
    }
}