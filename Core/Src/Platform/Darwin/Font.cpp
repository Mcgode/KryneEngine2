/**
 * @file
 * @author Max Godefroy
 * @date 13/01/2026.
 */

#include "KryneEngine/Core/Platform/Platform.hpp"

#include <CoreText/CoreText.h>
#include <EASTL/fixed_vector.h>

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
        eastl::fixed_vector<UniChar, 2, false> utf16;
        if (_unicodeCodePoint < 0xD800 || (_unicodeCodePoint > 0xDFFF && _unicodeCodePoint < 0x10000))
        {
            utf16.push_back(static_cast<UniChar>(_unicodeCodePoint));
        }
        else
        {
            const u32 cp = _unicodeCodePoint - 0x10000;
            utf16.push_back(static_cast<UniChar>(0xD800 + (cp >> 10)));
            utf16.push_back(static_cast<UniChar>(0xDC00 + (cp & 0x3FF)));
        }

        const CTFontRef defaultFont = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 0.f, nullptr);
        const CFArrayRef cascadeDescriptors = CTFontCopyDefaultCascadeListForLanguages(defaultFont, nullptr);
        if (cascadeDescriptors == nullptr) return false;

        bool found = false;
        const CFIndex cascadeCount = CFArrayGetCount(cascadeDescriptors);
        for (CFIndex i = 0; i < cascadeCount; ++i)
        {
            auto descriptor = static_cast<CTFontDescriptorRef>(CFArrayGetValueAtIndex(cascadeDescriptors, i));
            if (descriptor == nullptr)
                continue;

            const CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 0.f, nullptr);
            if (font == nullptr) continue;

            eastl::fixed_vector<CGGlyph, 2, false> glyphs(utf16.size());
            const bool ok = CTFontGetGlyphsForCharacters(font, utf16.data(), glyphs.data(), glyphs.size());

            if (ok && glyphs.front() != 0)
            {
                found = true;

                const double scale = CTFontGetUnitsPerEm(font) / CTFontGetSize(font);

                CGPathRef outline = CTFontCreatePathForGlyph(font, glyphs.front(), nullptr);
                if (outline != nullptr)
                {
                    const FontMetrics fontMetrics {
                        .m_ascender = CTFontGetAscent(font) * scale,
                        .m_descender = CTFontGetDescent(font) * scale,
                        .m_lineHeight = scale * (CTFontGetLeading(font) + CTFontGetAscent(font) + CTFontGetDescent(font)),
                        .m_unitPerEm = static_cast<double>(CTFontGetUnitsPerEm(font))
                    };

                    const CTFontOrientation orientation = _verticalLayout ? kCTFontOrientationVertical : kCTFontOrientationHorizontal;

                    CGSize advances;
                    CTFontGetAdvancesForGlyphs(font, orientation, glyphs.data(), &advances, 1);

                    CGRect bbox;
                    CTFontGetBoundingRectsForGlyphs(font, orientation, glyphs.data(), &bbox, 1);

                    const GlyphMetrics glyphMetrics {
                        .m_bounds = {
                            bbox.origin.x * scale,
                            bbox.origin.y * scale,
                            bbox.size.width * scale,
                            bbox.size.height * scale },
                        .m_advance = (_verticalLayout ? advances.height : advances.width) * scale,
                    };

                    _fontMetrics(fontMetrics, glyphMetrics, _userData);

                    struct Context
                    {
                        void* m_userData;
                        double scale;
                        FontNewContourFunction m_newContour;
                        FontNewEdgeFunction m_newEdge;
                        FontNewConicFunction m_newConic;
                        FontNewCubicFunction m_newCubic;
                        FontEndContourFunction m_endContour;
                        CGPoint m_firstPoint {};
                    };
                    Context context { _userData, scale, _newContour, _newEdge, _newConic, _newCubic, _endContour };
                    constexpr auto applier = [](void* _info, const CGPathElement* _element)
                    {
                        auto& context = *static_cast<Context*>(_info);

                        switch (_element->type)
                        {

                        case kCGPathElementMoveToPoint:
                            context.m_firstPoint = _element->points[0];
                            context.m_newContour(
                                { _element->points[0].x * context.scale, _element->points[0].y * context.scale },
                                context.m_userData);
                            break;
                        case kCGPathElementAddLineToPoint:
                            context.m_newEdge(
                                { _element->points[0].x * context.scale, _element->points[0].y * context.scale },
                                context.m_userData);
                            break;
                        case kCGPathElementAddQuadCurveToPoint:
                            context.m_newConic(
                                { _element->points[0].x * context.scale, _element->points[0].y * context.scale },
                                { _element->points[1].x * context.scale, _element->points[1].y * context.scale },
                                context.m_userData);
                            break;
                        case kCGPathElementAddCurveToPoint:
                            context.m_newCubic(
                                { _element->points[0].x * context.scale, _element->points[0].y * context.scale },
                                { _element->points[1].x * context.scale, _element->points[1].y * context.scale },
                                { _element->points[2].x * context.scale, _element->points[2].y * context.scale },
                                context.m_userData);
                            break;
                        case kCGPathElementCloseSubpath:
                            context.m_endContour(context.m_userData);
                            break;
                        }
                    };

                    CGPathApply(outline, &context, applier);
                }

                CFRelease(font);
                break;
            }

            CFRelease(font);
        }

        CFRelease(cascadeDescriptors);
        CFRelease(defaultFont);
        return found;
    }
}