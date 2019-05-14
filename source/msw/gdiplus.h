#pragma once

#include <stdlib.h>
#include <malloc.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>

/* manually declare everything GDI+ needs, because
   GDI+ headers are not usable from C */
#define WINGDIPAPI __stdcall
#define GDIPCONST const

typedef struct GpGraphics GpGraphics;
typedef struct GpImage GpImage;
typedef struct GpPen GpPen;
typedef struct GpBrush GpBrush;
typedef struct GpStringFormat GpStringFormat;
typedef struct GpFont GpFont;
typedef struct GpFontFamily GpFontFamily;
typedef struct GpFontCollection GpFontCollection;

typedef GpImage GpBitmap;
typedef GpBrush GpSolidFill;

typedef int Status;
typedef Status GpStatus;

typedef float REAL;
typedef DWORD ARGB;
typedef POINT GpPoint;

typedef enum {
	TextRenderingHintSystemDefault = 0,
	TextRenderingHintSingleBitPerPixelGridFit = 1,
	TextRenderingHintSingleBitPerPixel = 2,
	TextRenderingHintAntiAliasGridFit = 3,
	TextRenderingHintAntiAlias = 4,
	TextRenderingHintClearTypeGridFit = 5
} TextRenderingHint;

typedef enum {
	StringFormatFlagsDirectionRightToLeft    = 0x00000001,
	StringFormatFlagsDirectionVertical       = 0x00000002,
	StringFormatFlagsNoFitBlackBox           = 0x00000004,
	StringFormatFlagsDisplayFormatControl    = 0x00000020,
	StringFormatFlagsNoFontFallback          = 0x00000400,
	StringFormatFlagsMeasureTrailingSpaces   = 0x00000800,
	StringFormatFlagsNoWrap                  = 0x00001000,
	StringFormatFlagsLineLimit               = 0x00002000,
	StringFormatFlagsNoClip                  = 0x00004000 
} StringFormatFlags;

typedef enum
{
	QualityModeInvalid   = -1,
	QualityModeDefault   = 0,
	QualityModeLow       = 1,
	QualityModeHigh      = 2
} QualityMode;

typedef enum
{
	SmoothingModeInvalid     = QualityModeInvalid,
	SmoothingModeDefault     = QualityModeDefault,
	SmoothingModeHighSpeed   = QualityModeLow,
	SmoothingModeHighQuality = QualityModeHigh,
	SmoothingModeNone,
	SmoothingModeAntiAlias,
	SmoothingModeAntiAlias8x4 = SmoothingModeAntiAlias,
	SmoothingModeAntiAlias8x8
} SmoothingMode;

typedef enum
{
	FontStyleRegular    = 0,
	FontStyleBold       = 1,
	FontStyleItalic     = 2,
	FontStyleBoldItalic = 3,
	FontStyleUnderline  = 4,
	FontStyleStrikeout  = 8
} FontStyle;

typedef enum {
	FillModeAlternate,
	FillModeWinding
} FillMode;

typedef enum {
	CombineModeReplace,
	CombineModeIntersect,
	CombineModeUnion,
	CombineModeXor,
	CombineModeExclude,
	CombineModeComplement
} CombineMode;

typedef enum {
	UnitWorld,
	UnitDisplay,
	UnitPixel,
	UnitPoint,
	UnitInch,
	UnitDocument,
	UnitMillimeter
} Unit;

typedef struct {
	FLOAT X;
	FLOAT Y;
	FLOAT Width;
	FLOAT Height;
} RectF;

typedef enum {
	DebugEventLevelFatal,
	DebugEventLevelWarning
} DebugEventLevel;

typedef VOID (WINAPI *DebugEventProc)(DebugEventLevel level, CHAR *message);

typedef struct {
	UINT32 GdiplusVersion;
	DebugEventProc DebugEventCallback;
	BOOL SuppressBackgroundThread;
	BOOL SuppressExternalCodecs;
} GdiplusStartupInput;

typedef Status (WINAPI *NotificationHookProc)(OUT ULONG_PTR *token);
typedef VOID (WINAPI *NotificationUnhookProc)(ULONG_PTR token);

typedef struct {
	NotificationHookProc NotificationHook;
	NotificationUnhookProc NotificationUnhook;
} GdiplusStartupOutput;

/* startup & shutdown */

Status WINAPI GdiplusStartup(OUT ULONG_PTR *token,
	const GdiplusStartupInput *input,
	OUT GdiplusStartupOutput *output);

VOID WINAPI GdiplusShutdown(ULONG_PTR token);

/* image */

GpStatus WINGDIPAPI
GdipCreateBitmapFromGraphics(INT width,
	INT height, GpGraphics* target, GpBitmap** bitmap);

GpStatus WINGDIPAPI
GdipDisposeImage(GpImage *image);

GpStatus WINGDIPAPI
GdipGetImageGraphicsContext(GpImage *image, GpGraphics **graphics);

GpStatus WINGDIPAPI
GdipGetImageWidth(GpImage *image, UINT *width);

GpStatus WINGDIPAPI
GdipGetImageHeight(GpImage *image, UINT *height);

GpStatus WINGDIPAPI
GdipLoadImageFromFile(GDIPCONST WCHAR* filename, GpImage **image);

GpStatus WINGDIPAPI
GdipLoadImageFromStream(IStream* stream, GpImage **image);

/* pen */

GpStatus WINGDIPAPI
GdipCreatePen1(ARGB color, REAL width, Unit unit, GpPen **pen);

GpStatus WINGDIPAPI
GdipDeletePen(GpPen *pen);

GpStatus WINGDIPAPI
GdipSetPenWidth(GpPen *pen, REAL width);

GpStatus WINGDIPAPI
GdipSetPenColor(GpPen *pen, ARGB argb);

/* brush */

GpStatus WINGDIPAPI
GdipCreateSolidFill(ARGB color, GpSolidFill **brush);

GpStatus WINGDIPAPI
GdipDeleteBrush(GpBrush *brush);

GpStatus WINGDIPAPI
GdipSetSolidFillColor(GpSolidFill *brush, ARGB color);

/* font */

GpStatus WINGDIPAPI
GdipCreateFont(GDIPCONST GpFontFamily *fontFamily, REAL emSize,
	INT style, Unit unit, GpFont **font);

GpStatus WINGDIPAPI
GdipDeleteFont(GpFont* font);

GpStatus WINGDIPAPI
GdipGetFontSize(GpFont *font, REAL *size);

GpStatus WINGDIPAPI
GdipCreateFontFamilyFromName(GDIPCONST WCHAR *name,
	GpFontCollection *fontCollection,
	GpFontFamily **fontFamily);

GpStatus WINGDIPAPI
GdipDeleteFontFamily(GpFontFamily *fontFamily);

GpStatus WINGDIPAPI
GdipStringFormatGetGenericTypographic(GpStringFormat **format);

GpStatus WINGDIPAPI
GdipSetStringFormatFlags(GpStringFormat *format, INT flags);

GpStatus WINGDIPAPI
GdipDeleteStringFormat(GpStringFormat *format);

GpStatus WINGDIPAPI
GdipPrivateAddMemoryFont(GpFontCollection *fontCollection,
	GDIPCONST void *memory, INT length);

GpStatus WINGDIPAPI
GdipPrivateAddFontFile(GpFontCollection *fontCollection,
	GDIPCONST WCHAR *filename);

GpStatus WINGDIPAPI
GdipNewPrivateFontCollection(GpFontCollection **fontCollection);

GpStatus WINGDIPAPI
GdipDeletePrivateFontCollection(GpFontCollection **fontCollection);

GpStatus WINGDIPAPI
GdipGetFontCollectionFamilyList(GpFontCollection* fontCollection,
	INT numSought, GpFontFamily *gpfamilies[], INT *numFound);

GpStatus WINGDIPAPI
GdipGetFontCollectionFamilyCount(GpFontCollection *fontCollection,
	INT *numFound);

/* graphics */

GpStatus WINGDIPAPI
GdipCreateFromHWND(HWND hwnd, GpGraphics **graphics);

GpStatus WINGDIPAPI
GdipCreateFromHDC(HDC hdc, GpGraphics **graphics);

GpStatus WINGDIPAPI
GdipDeleteGraphics(GpGraphics *graphics);

GpStatus WINGDIPAPI
GdipSetSmoothingMode(GpGraphics *graphics, SmoothingMode smoothingMode);

GpStatus WINGDIPAPI
GdipSetClipRectI(GpGraphics *graphics, INT x, INT y,
	INT width, INT height, CombineMode combineMode);

GpStatus WINGDIPAPI
GdipDrawLineI(GpGraphics *graphics, GpPen *pen, INT x1, INT y1,
	INT x2, INT y2);

GpStatus WINGDIPAPI
GdipDrawArcI(GpGraphics *graphics, GpPen *pen, INT x, INT y,
	INT width, INT height, REAL startAngle, REAL sweepAngle);

GpStatus WINGDIPAPI
GdipFillPieI(GpGraphics *graphics, GpBrush *brush, INT x, INT y,
	INT width, INT height, REAL startAngle, REAL sweepAngle);

GpStatus WINGDIPAPI
GdipDrawRectangleI(GpGraphics *graphics, GpPen *pen, INT x, INT y,
	INT width, INT height);

GpStatus WINGDIPAPI
GdipFillRectangleI(GpGraphics *graphics, GpBrush *brush, INT x, INT y,
	INT width, INT height);

GpStatus WINGDIPAPI
GdipFillPolygonI(GpGraphics *graphics, GpBrush *brush,
	GDIPCONST GpPoint *points, INT count, FillMode fillMode);

GpStatus WINGDIPAPI
GdipDrawPolygonI(GpGraphics *graphics, GpPen *pen, GDIPCONST GpPoint *points,
	INT count);

GpStatus WINGDIPAPI
GdipFillEllipseI(GpGraphics *graphics, GpBrush *brush, INT x, INT y,
	INT width, INT height);

GpStatus WINGDIPAPI
GdipDrawEllipseI(GpGraphics *graphics, GpPen *pen, INT x, INT y,
	INT width, INT height);

GpStatus WINGDIPAPI
GdipDrawBezierI(GpGraphics *graphics, GpPen *pen, INT x1, INT y1,
	INT x2, INT y2, INT x3, INT y3, INT x4, INT y4);

GpStatus WINGDIPAPI
GdipDrawString(GpGraphics *graphics, GDIPCONST WCHAR *string, INT length,
	GDIPCONST GpFont *font, GDIPCONST RectF *layoutRect,
	GDIPCONST GpStringFormat *stringFormat, GDIPCONST GpBrush *brush);

GpStatus WINGDIPAPI
GdipGraphicsClear(GpGraphics *graphics, ARGB color);

GpStatus WINGDIPAPI
GdipDrawImageI(GpGraphics *graphics, GpImage *image, INT x, INT y);

GpStatus WINGDIPAPI
GdipDrawImageRectI(GpGraphics *graphics, GpImage *image, INT x, INT y,
	INT width, INT height);

GpStatus WINGDIPAPI
GdipMeasureString(
    GpGraphics               *graphics,
    GDIPCONST WCHAR          *string,
    INT                       length,
    GDIPCONST GpFont         *font,
    GDIPCONST RectF          *layoutRect,
    GDIPCONST GpStringFormat *stringFormat,
    RectF                    *boundingBox,
    INT                      *codepointsFitted,
    INT                      *linesFilled
);

GpStatus WINGDIPAPI
GdipSetTextRenderingHint(GpGraphics *graphics, TextRenderingHint mode);

LWSTDAPI_(IStream *) SHCreateMemStream(const BYTE *pInit, _In_ UINT cbInit);
