#include "hmain.hpp"

#include "fonts.h"
#include "mcufont.h"

void DrawCircle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> radius, uint8_t rotation)
{
    int mx = std::max(radius.x, radius.y);

    if(radius.x == radius.y || rotation == 0)
    {
        int lmxx = std::min(pos.x+radius.x, context.screen_size.x);
        int lmxy = std::min(pos.y+radius.y, context.screen_size.y);
        int lmnx = std::max(pos.x-radius.x, 0);
        int lmny = std::max(pos.y-radius.y, 0);

        if(radius.x == radius.y)
        {
            for(int i = lmnx; i < lmxx; i++)
            {
                for(int ii = lmny; ii < lmxy; ii++)
                {
                    int dist = Distance(float(i), float(ii), float(pos.x), float(pos.y));
                    if(dist <= mx)
                    {
                        if(dist == mx || dist == mx-1)
                            SetPixel(context, border, {i, ii});
                        else 
                            SetPixel(context, fill, {i, ii});
                    }    
                }
            }
        }
        else
        {
            for(int i = lmnx; i < lmxx; i++)
            {
                for(int ii = lmny; ii < lmxy; ii++)
                {
                    float d = SQ(float(i)-float(pos.x)) / SQ(float(radius.x)) + SQ(float(ii)-float(pos.y)) / SQ(float(radius.y));
                    if(d <= 1)
                    {
                        if(d < 0.99)
                            SetPixel(context, fill, {i, ii});   
                        else
                            SetPixel(context, border, {i, ii});
                    }
                }
            }
        }
    }
    else
    {
        float rot = M_PI*float(rotation)/255.0f;

        int lmxx = std::min(pos.x+mx, context.screen_size.x);
        int lmxy = std::min(pos.y+mx, context.screen_size.y);
        int lmnx = std::max(pos.x-mx, 0);
        int lmny = std::max(pos.y-mx, 0);

        for(int i = lmnx; i < lmxx; i++)
            {
                for(int ii = lmny; ii < lmxy; ii++)
                {
                    float d = SQ((float(i)-float(pos.x))*cos(rot) + (float(ii)-float(pos.y))*sin(rot))/SQ(float(radius.x)) +
                        SQ((float(i)-float(pos.x))*sin(rot) - (float(ii)-float(pos.y))*cos(rot))/SQ(float(radius.y));
                    if(d <= 1)
                    {
                        if(d < 0.99)
                            SetPixel(context, fill, {i, ii});   
                        else
                            SetPixel(context, border, {i, ii});
                    }
                }
            }
    }
}
void DrawEmptyCircle(const ScreenContext& context, uint8_t color, vec2<int> pos, vec2<int> size, uint8_t rotation)
{
    if(rotation == 0)
    {
        for(int i = 0; i < size.x; i++)
        {
            if(pos.x + size.x >= context.screen_size.x)
                break;
            if(pos.x+i < 0)
                continue;
            int sy = size.y/2;
            int px = i - sy;
            int p = sqrt(sy*sy - px*px);
            int pt = pos.y + p;
            int pb = pos.y - p;
            if(pt >= 0 && pt < context.screen_size.y)
                SetPixel(context, color, {i+pos.x, pt});
            if(pb >= 0 && pt < context.screen_size.y)
                SetPixel(context, color, {i+pos.x, pb});
        }
    }    
}
vec2<int> RotatePoint(vec2<int> point, vec2<int> center, float rotation)
{
    float sint = sin(rotation);
    float cost = cos(rotation);
    return 
    {
        cost*(point.x - center.x) - sint * (point.y - center.y) + center.x,
        sint * (point.x - center.x) + cost * (point.y - center.y) + center.y
    };
}
vec2<int> RotatePoint(vec2<int> point, vec2<int> center, uint8_t rotation)
{
    return RotatePoint(point, center, float(M_PI*float(rotation)/255.0f));
}
void DrawRectangle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> size, uint8_t rotation)
{
    if(rotation == 0)
    {
        if(size.x > 2 && size.y > 2)
        {
            for(int i = std::max(0, pos.x-size.x/2); i < std::min(context.screen_size.x, pos.x+size.x/2); i++)
            {
                for(int ii = std::max(0, pos.y-size.y/2); ii < std::min(context.screen_size.y, pos.y+size.y/2); ii++)
                {
                    SetPixel(context, fill, {i, ii});
                }
            }
        }
        DrawEmptyRectangle(context, border, pos, size, rotation);
    }
    else
    {
        if(size.x <= 2 || size.y <= 2)
            return;

        float rot = M_PI*float(rotation)/255.0f;

        vec2<int> TL = {pos.x - size.x/2, pos.y - size.y/2};
        vec2<int> TR = {pos.x + size.x/2, pos.y - size.y/2};
        vec2<int> BL = {pos.x - size.x/2, pos.y + size.y/2};
        vec2<int> BR = {pos.x + size.x/2, pos.y + size.y/2 };

        TL = RotatePoint(TL, pos, rotation);
        TR = RotatePoint(TR, pos, rotation);
        BL = RotatePoint(BL, pos, rotation);
        BR = RotatePoint(BR, pos, rotation);

        
        DrawTriangle(context, fill, {0,0}, {1,1}, TL, TR, BR, false, 0);
        DrawTriangle(context, fill, {0,0}, {1,1}, TL, BL, BR, false, 0);
        DrawEmptyRectangle(context, border, pos, size, rotation);
    }
}
void DrawEmptyRectangle(const ScreenContext& context, uint8_t color, vec2<int> pos, vec2<int> size, uint8_t rotation)
{
    if(rotation == 0)
    {
        for(int i = std::max(pos.x-size.x/2, 0); i < std::min(pos.x+size.x/2, context.screen_size.x); i++)
            SetPixel(context, color, {i, pos.y-size.y/2});
        for(int i = std::max(pos.x-size.x/2, 0); i < std::min(pos.x+size.x/2, context.screen_size.x); i++)
            SetPixel(context, color, {i, pos.y+size.y/2});
        for(int i = std::max(pos.y-size.y/2, 0); i < std::min(pos.y+size.y/2, context.screen_size.y); i++)
            SetPixel(context, color, {pos.x-size.x/2, i});
        for(int i = std::max(pos.y-size.y/2, 0); i < std::min(pos.y+size.y/2, context.screen_size.y); i++)
            SetPixel(context, color, {pos.x+size.x/2, i});
    }
    else
    {
        vec2<int> TL = {pos.x - size.x/2, pos.y - size.y/2};
        vec2<int> TR = {pos.x + size.x/2, pos.y - size.y/2};
        vec2<int> BL = {pos.x - size.x/2, pos.y + size.y/2};
        vec2<int> BR = {pos.x + size.x/2, pos.y + size.y/2 };

        TL = RotatePoint(TL, pos, rotation);
        TR = RotatePoint(TR, pos, rotation);
        BL = RotatePoint(BL, pos, rotation);
        BR = RotatePoint(BR, pos, rotation);

        DrawLine(context, color, TL, TR, 0);
        DrawLine(context, color, TR, BR, 0);
        DrawLine(context, color, BR, BL, 0);
        DrawLine(context, color, BL, TL, 0);
    }
}

int TriangleArea(int x1, int y1, int x2, int y2, int x3, int y3)
{
   return abs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))/2);
}

bool IsInsideTriangle(int A, int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
{   
   int A1 = TriangleArea (x, y, x2, y2, x3, y3);
   int A2 = TriangleArea (x1, y1, x, y, x3, y3);
   int A3 = TriangleArea (x1, y1, x2, y2, x, y);
   int sum = A1 + A2 + A3;
   return (abs(A - sum) < 2);
}

bool IsTriangleClockwise(vec2<int> p0, vec2<int> p1, vec2<int> p2)
{
    return ((p1.x-p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x)) < 0;
}
int edgeFunction(vec2<int> a, vec2<int> b, vec2<int> c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
void DrawTriangle(const ScreenContext& context, uint8_t fill, vec2<int> pos, vec2<int> size, vec2<int> p0, vec2<int> p1, vec2<int> p2, bool centertriangle, uint8_t rotation)
{
    if(edgeFunction(p0, p1, p2) < 0)
    {
        std::swap(p1, p2);
        //std::swap(p1, p2);
    }
    printf("%i\n", edgeFunction(p0, p1, p2));

    p0 += pos;
    p1 += pos;
    p2 += pos;

    vec2<int> center = (p0+p1+p2)/vec2<int>{3, 3};
    p0 = ((p0-center)*size) + center;
    p1 = ((p1-center)*size) + center;
    p2 = ((p2-center)*size) + center;

    if(rotation != 0)
    {
        p0 = RotatePoint(p0, center, rotation);
        p1 = RotatePoint(p1, center, rotation);
        p2 = RotatePoint(p2, center, rotation);
    }


    vec2<int> topl = {std::min({p0.x, p1.x, p2.x}), std::min({p0.y, p1.y, p2.y})};
    vec2<int> botr = {std::max({p0.x, p1.x, p2.x}), std::max({p0.y, p1.y, p2.y})};
    

    if(centertriangle)
    {
        vec2<int> dif = (botr-topl)/vec2<int>{2, 2};
        topl -= dif;
        botr -= dif;
        p0 -= dif;
        p1 -= dif; 
        p2 -= dif;
    }
    for(int y = std::max(0, topl.y); y < std::min(botr.y, context.screen_size.y); y++)
    {
        for(int x = std::max(0, topl.x); x < std::min(botr.x, context.screen_size.x); x++)
        {

            vec2<int> p = {x, y};
            int ABP = edgeFunction(p0, p1, p);
            int BCP = edgeFunction(p1, p2, p);
            int CAP = edgeFunction(p2, p0, p);
            if(ABP >= 0 && BCP >= 0 && CAP >= 0)
            {
                SetPixel(context, fill, {x, y});
            }
        }
    }
}
void DrawLine(const ScreenContext& context, uint8_t fill, vec2<int> p0, vec2<int> p1, uint8_t rotation)
{
    if(rotation != 0)
    {
        float rotation = M_PI*float(rotation)/255.0f;
        vec2<int> center = (p0+p1)/vec2<int>{2,2};
        p0 = RotatePoint(p0, center, rotation);
        p1 = RotatePoint(p1, center, rotation);
    }
    p0 = p0.min(context.screen_size).max(0);
    p1 = p1.min(context.screen_size).max(0);
    vec2<int> dif = p1-p0;
    int len = sqrt(dif.x*dif.x + dif.y*dif.y);

    for(int i = 0; i < len; i++)
    {
        vec2<int> off;
        off.x = dif.x*i/len;
        off.y = dif.y*i/len;
        vec2<int> pos = p0 + off;
        SetPixel(context, fill, pos);
    }
    
}

const mf_font_s* current_font;
void PixelCallback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void* state)
{
    ScreenContext* context = (ScreenContext*)state;
    for(int i = 0; i < count; i++)
    {
        context->data[y * context->screen_size.x + x + i] = 255-alpha;
    }
}
uint8_t CharacterCallback(int16_t x, int16_t y, mf_char character, void* state)
{
    return mf_render_character(current_font, x, y, character, &PixelCallback, state);
}

void DrawText(const ScreenContext& context, vec2<uint16_t> pos, FONT font, uint16_t bufstart, uint16_t len, TEXT_ALIGNMENT alignment)
{
    static const mf_font_s* fonts[] = {&mf_rlefont_DejaVuSans12.font, &mf_rlefont_DejaVuSans12bw.font, 
        &mf_rlefont_DejaVuSerif16.font, &mf_rlefont_DejaVuSerif32.font, &mf_bwfont_fixed_5x8.font,
        &mf_rlefont_fixed_7x14.font, &mf_rlefont_fixed_10x20.font};
    if((uint8_t)font >= ArraySize(fonts))
    {
        printf("Invalid font %i\n", (int)font);
        return;
    }

    if(bufstart + len > TEXT_BUFFER_SIZE)
    {
        printf("Out of text buffer range at %i for len %i\n", (int)bufstart, (int)len);
        return;
    }

    current_font = fonts[(int)font];

    mf_render_aligned(current_font, pos.x, pos.y, (mf_align_t)alignment, text_buffer + bufstart, len, &CharacterCallback, (void*)&context);
}
void DrawSprite(const ScreenContext& context, vec2<uint16_t> pos16, vec2<uint16_t> size16, uint32_t start, uint32_t len, bool center)
{
    vec2<int> pos = {pos16.x, pos16.y};
    vec2<int> size = {size16.x, size16.y};
    if(center)
        pos -= size/vec2<int>{2, 2};
    if(pos.x >= context.screen_size.x || pos.y >= context.screen_size.y)
        return;
    pos = pos.max(0);
    int hsize = size.x;
    if(pos.x+size.x > context.screen_size.x)
        hsize += context.screen_size.x - (pos.x+size.x);
    for(int i = pos.y; i < std::min(context.screen_size.y, pos.y+size.y); i++)
    {
        memcpy(context.data + i*context.screen_size.x + pos.x, texture_buffer + start + ((i-pos.y)*size.x), hsize);
    }
}

void DrawEntity(const Entity& entity, const ScreenContext& context)
{
    switch(entity.type)
    {
        case ENTITY_TYPE::SHAPE:
        {
            switch((SHAPE)entity.data[0])
            {
                case SHAPE::CIRCLE:
                {
                    DrawCircle(context, entity.data[1], entity.data[2], {entity.pos.x, entity.pos.y}, 
                        {entity.size.x, entity.size.y}, entity.rotation);
                    break;
                }
                case SHAPE::EMPTY_CIRCLE:
                {
                    DrawEmptyCircle(context, entity.data[1], {entity.pos.x, entity.pos.y}, {entity.size.x, entity.size.y}, entity.rotation);
                    break;
                }
                case SHAPE::RECTANGLE:
                {
                    DrawRectangle(context, entity.data[1], entity.data[2], {entity.pos.x, entity.pos.y}, 
                        {entity.size.x, entity.size.y}, entity.rotation);
                    break;
                }
                case SHAPE::EMPTY_RECTANGLE:
                {
                    DrawEmptyRectangle(context, entity.data[1], {entity.pos.x, entity.pos.y}, {entity.size.x, entity.size.y}, entity.rotation);
                    break;
                }
                case SHAPE::TRIANGLE:
                {
                    DrawTriangle(context, entity.data[1], {entity.pos.x, entity.pos.y}, 
                        {entity.size.x, entity.size.y}, {entity.data[2], entity.data[3]}, 
                        {entity.data[4], entity.data[5]}, {entity.data[6], entity.data[7]}, entity.data[8], entity.rotation);
                    break;
                }
                default:
                { 
                    printf("Invalid shape!: %i\n", (int)entity.data[0]);
                }
            }
            break;
        };
        case ENTITY_TYPE::LINE:
        {
            DrawLine(context, entity.data[0], {entity.pos.x, entity.pos.y}, {entity.size.x, entity.size.y}, entity.rotation);
            break;
        }
        case ENTITY_TYPE::TEXT:
        {
            DrawText(context, entity.pos, (FONT)entity.data[0], *(uint16_t*)(entity.data + 1), *(uint16_t*)(entity.data + 3), (TEXT_ALIGNMENT)*(entity.data + 5));
            break;
        }
        case ENTITY_TYPE::POINT:
        {
            SetPixelSafe(context, entity.data[0], {entity.pos.x, entity.pos.y});
            break;
        }
        case ENTITY_TYPE::SPRITE:
        {
            DrawSprite(context, {entity.pos.x, entity.pos.y}, {entity.pos.x, entity.pos.y}, *(uint32_t*)entity.data, *(uint32_t*)(entity.data + 4), entity.data + 8);
            break;
        }

        default:
        {
            printf("Invalid entity type!: %i\n", (int)entity.type);
        }
    }
}