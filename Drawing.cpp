#include "hmain.hpp"

#include "fonts.h"
#include "mcufont.h"

void DrawCircle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> radius, uint8_t rotation, bool center)
{
    int mx = std::max(radius.x, radius.y);
    int mxsq = SQ(mx);

    if(!center)
    {
        pos += radius;
    }
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
                    int dist = DistanceSQ(float(i), float(ii), float(pos.x), float(pos.y));
                    if(dist <= mxsq)
                    {
                        if(dist == mxsq || dist == mxsq-1)
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
void DrawEmptyCircle(const ScreenContext& context, uint8_t color, vec2<int> pos, vec2<int> size, uint8_t mode, bool center)
{
    size *= 2;
    if(!center)
    {
        pos += size/2;
    }
    pos.x -= size.x/2;
    for(int i = size.x/8; i < size.x*7/8; i++)
    {
        int sy = size.y/2;
        int px = i - sy;
        int p = sqrt(sy*sy - px*px);
        int pt = pos.y - p;
        int pb = pos.y + p;
        if(mode == 0 || mode == 1)
        {
            if(pt >= 0 && pt < context.screen_size.y)
                SetPixelSafe(context, color, {i+pos.x, pt});
        }
        if(mode == 0 || mode == 2)
        {
            if(pb >= 0 && pt < context.screen_size.y)
                SetPixelSafe(context, color, {i+pos.x, pb});
        }
    }
    pos.x += size.x/2;
    pos.y -= size.y/2;
    for(int i = size.y/8; i < size.y*7/8; i++)
    {
        int sy = size.x/2;
        int px = i - sy;
        int p = sqrt(sy*sy - px*px);
        int pt = pos.x - p;
        int pb = pos.x + p;
        if(mode == 0 || mode == 1)
        {
            if(pt >= 0 && pt < context.screen_size.y)
                SetPixelSafe(context, color, {pt, i+pos.y});
        }
        if(mode == 0 || mode == 2)
        {
            if(pb >= 0 && pt < context.screen_size.y)
                SetPixelSafe(context, color, {pb, i+pos.y});
        }
    }
}
vec2<int> RotatePoint(vec2<int> point, vec2<int> center, float rotation)
{
    float sint = sin(rotation);
    float cost = cos(rotation);
    return 
    {
        int(cost * float(point.x - center.x) - sint * float(point.y - center.y)) + center.x,
        int(sint * float(point.x - center.x) + cost * float(point.y - center.y)) + center.y
    };
}
vec2<int> RotatePoint(vec2<int> point, vec2<int> center, uint8_t rotation)
{
    return RotatePoint(point, center, 2.0f*float(M_PI)*float(rotation)/255.0f);
}
void DrawRectangle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> size, uint8_t rotation, bool center)
{
    if(rotation == 0)
    {
        if(size.x > 2 && size.y > 2)
        {
            if(center)
            {
                for(int i = std::max(0, pos.x-size.x/2); i < std::min(context.screen_size.x, pos.x+size.x/2); i++)
                {
                    for(int ii = std::max(0, pos.y-size.y/2); ii < std::min(context.screen_size.y, pos.y+size.y/2); ii++)
                    {
                        SetPixel(context, fill, {i, ii});
                    }
                }
            }
            else
            {
                for(int i = std::max(0, pos.y); i < std::min(context.screen_size.y, pos.y+size.y); i++)
                {
                    int start = std::max(0, pos.x);
                    int end = std::min(context.screen_size.x, pos.x+size.x);
                    memset(context.data + i*context.screen_size.y + start, fill, end-start);
                }
            }
        }
        DrawEmptyRectangle(context, border, pos, size, rotation, center);
    }
    else
    {
        if(size.x <= 2 || size.y <= 2)
            return;

        float rot = M_PI*float(rotation)/255.0f;

        vec2<int> TL;
        vec2<int> TR;
        vec2<int> BL;
        vec2<int> BR;
        
        if(center)
        {
            TL = {pos.x - size.x/2, pos.y - size.y/2};
            TR = {pos.x + size.x/2, pos.y - size.y/2};
            BL = {pos.x - size.x/2, pos.y + size.y/2};
            BR = {pos.x + size.x/2, pos.y + size.y/2 };
        }
        else
        {
            TL = {pos.x, pos.y};
            TR = {pos.x + size.x, pos.y};
            BL = {pos.x, pos.y + size.y};
            BR = {pos.x + size.x, pos.y + size.y};
        }

        vec2<int> center = (TL+TR+BL+BR)/4;

        TL = RotatePoint(TL, center, rotation);
        TR = RotatePoint(TR, center, rotation);
        BL = RotatePoint(BL, center, rotation);
        BR = RotatePoint(BR, center, rotation);

        
        DrawTriangle(context, fill, {0,0}, {1,1}, TL, TR, BR, false, 0);
        DrawTriangle(context, fill, {0,0}, {1,1}, TL, BL, BR, false, 0);
        DrawEmptyRectangle(context, border, pos, size, rotation, true);
    }
}
void DrawEmptyRectangle(const ScreenContext& context, uint8_t color, vec2<int> pos, vec2<int> size, uint8_t rotation, bool center)
{
    if(rotation == 0)
    {
        if(center)
        {
            int boty = std::max(std::min(context.screen_size.y, pos.y-size.y/2), 0);
            int topy = std::max(std::min(context.screen_size.y, pos.y+size.y/2), 0);
            int botx = std::max(std::min(context.screen_size.x, pos.x-size.x/2), 0);
            int topx = std::max(std::min(context.screen_size.x, pos.x+size.x/2), 0);

            for(int i = std::max(pos.x-size.x/2, 0); i < std::min(pos.x+size.x/2, context.screen_size.x); i++)
                SetPixel(context, color, {i, boty});
            for(int i = std::max(pos.x-size.x/2, 0); i < std::min(pos.x+size.x/2, context.screen_size.x); i++)
                SetPixel(context, color, {i, topy});
            for(int i = std::max(pos.y-size.y/2, 0); i < std::min(pos.y+size.y/2, context.screen_size.y); i++)
                SetPixel(context, color, {botx, i});
            for(int i = std::max(pos.y-size.y/2, 0); i < std::min(pos.y+size.y/2, context.screen_size.y); i++)
                SetPixel(context, color, {topx, i});
        }
        else
        {
            int boty = std::max(std::min(context.screen_size.y, pos.y), 0);
            int topy = std::max(std::min(context.screen_size.y, pos.y+size.y), 0);
            int botx = std::max(std::min(context.screen_size.x, pos.x), 0);
            int topx = std::max(std::min(context.screen_size.x, pos.x+size.x), 0);

            for(int i = std::max(pos.x, 0); i < std::min(pos.x+size.x, context.screen_size.x); i++)
                SetPixel(context, color, {i, boty});
            for(int i = std::max(pos.x, 0); i < std::min(pos.x+size.x, context.screen_size.x); i++)
                SetPixel(context, color, {i, topy});
            for(int i = std::max(pos.y, 0); i < std::min(pos.y+size.y, context.screen_size.y); i++)
                SetPixel(context, color, {botx, i});
            for(int i = std::max(pos.y, 0); i < std::min(pos.y+size.y, context.screen_size.y); i++)
                SetPixel(context, color, {topx, i});            
        }
    }
    else
    {
        vec2<int> TL;
        vec2<int> TR;
        vec2<int> BL;
        vec2<int> BR;

        if(center)
        {
            TL = {pos.x - size.x/2, pos.y - size.y/2};
            TR = {pos.x + size.x/2, pos.y - size.y/2};
            BL = {pos.x - size.x/2, pos.y + size.y/2};
            BR = {pos.x + size.x/2, pos.y + size.y/2 };
        }
        else
        {
            TL = {pos.x, pos.y};
            TR = {pos.x + size.x, pos.y};
            BL = {pos.x, pos.y + size.y};
            BR = {pos.x + size.x, pos.y + size.y};
        }
        vec2<int> center = (TL+TR+BL+BR)/4;

        TL = RotatePoint(TL, center, rotation);
        TR = RotatePoint(TR, center, rotation);
        BL = RotatePoint(BL, center, rotation);
        BR = RotatePoint(BR, center, rotation);

        DrawLine(context, color, TL, TR, 0);
        DrawLine(context, color, TR, BR, 0);
        DrawLine(context, color, BR, BL, 0);
        DrawLine(context, color, BL, TL, 0);
    }
}

int Edge(vec2<int> a, vec2<int> b, vec2<int> c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
void DrawTriangle(const ScreenContext& context, uint8_t fill, vec2<int> pos, vec2<int> size, vec2<int> p0, vec2<int> p1, vec2<int> p2, bool centeraroundzero, uint8_t rotation)
{
    if(Edge(p0, p1, p2) < 0)
    {
        std::swap(p1, p2);
    }

    vec2<int> center = (p0+p1+p2)/3;

    if(rotation != 0)
    {
        p0 = RotatePoint(p0, center, rotation);
        p1 = RotatePoint(p1, center, rotation);
        p2 = RotatePoint(p2, center, rotation);
    }


    if(centeraroundzero)
    {
        p0 -= center;
        p1 -= center;
        p2 -= center;
        center = {0, 0};
    }

    p0 = ((p0-center)*size) + center;
    p1 = ((p1-center)*size) + center;
    p2 = ((p2-center)*size) + center;


    p0 += pos;
    p1 += pos;
    p2 += pos;

    vec2<int> topl = {std::min({p0.x, p1.x, p2.x}), std::min({p0.y, p1.y, p2.y})};
    vec2<int> botr = {std::max({p0.x, p1.x, p2.x}), std::max({p0.y, p1.y, p2.y})};
    
    for(int y = std::max(0, topl.y); y < std::min(botr.y, context.screen_size.y); y++)
    {
        for(int x = std::max(0, topl.x); x < std::min(botr.x, context.screen_size.x); x++)
        {
            vec2<int> p = {x, y};
            int ABP = Edge(p0, p1, p);
            int BCP = Edge(p1, p2, p);
            int CAP = Edge(p2, p0, p);
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
        vec2<int> center = (p0+p1)/2;
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
void DrawBezier(const ScreenContext& context, vec2<int> pos, uint8_t color, vec2<int>* points, int npoints, uint8_t rotation)
{
    vec2<int> center;
    if(rotation != 0)
    {
        for(int i = 0; i < npoints; i++)
        {
            center += points[i];
        }
        center = center / npoints;
    }
    for(int i = 0; i < npoints-2; i+=2)
    {
        vec2<int> p0 = points[i];
        vec2<int> p1 = points[i+1];
        vec2<int> p2 = points[i+2];

        if(rotation != 0)
        {
            p0 = RotatePoint(p0, center, rotation);
            p1 = RotatePoint(p1, center, rotation);
            p2 = RotatePoint(p2, center, rotation);
        }

        int len = Distance(p0, p1) + Distance(p1, p2) + Distance(p0, p2);
        auto interpolate = [](int a, int b, float t)->int
        {
            return a + (float(b-a)*t);
        };
        for(int i = 0; i < len; i++)
        {
            float t = float(i)/float(len);

            int xa = interpolate(p0.x, p1.x, t);
            int ya = interpolate(p0.y, p1.y, t);
            int xb = interpolate(p1.x, p2.x, t);
            int yb = interpolate(p1.y, p2.y, t);

            int x = interpolate(xa, xb, t);
            int y = interpolate(ya, yb, t);
            SetPixelSafe(context, color, {pos.x+x, pos.y+y});
        }
    }
}

static bool text_is_white;
static const mf_font_s* current_font;
void PixelCallback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void* state)
{
    ScreenContext* context = (ScreenContext*)state;
    for(int i = 0; i < count; i++)
    {
        context->data[y * context->screen_size.x + x + i] = (text_is_white) ? alpha : 255-alpha;
    }
}
uint8_t CharacterCallback(int16_t x, int16_t y, mf_char character, void* state)
{
    return mf_render_character(current_font, x, y, character, &PixelCallback, state);
}

void DrawText(const ScreenContext& context, vec2<uint16_t> pos, FONT font, uint16_t bufstart, uint16_t len, TEXT_ALIGNMENT alignment, bool white)
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
    text_is_white = white;

    mf_render_aligned(current_font, pos.x, pos.y, (mf_align_t)alignment, text_buffer + bufstart, len, &CharacterCallback, (void*)&context);
}
void DrawSprite(const ScreenContext& context, vec2<int> pos, vec2<int> size, uint8_t rotation, uint32_t start, uint32_t len, bool center, bool usetransparency, uint8_t transparencyvalue)
{
    if(center)
        pos -= size/2;
    if(rotation == 0)
    {
        if(!usetransparency)
        {
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
        else
        {
            for(int y = 0; y < size.y; y++)
            {
                for(int x = 0; x < size.x; x++)
                {
                    uint8_t v = *(texture_buffer + start + size.x*y + x);
                    if(v != transparencyvalue)
                        SetPixelSafe(context, v, pos + vec2<int>{x, y});
                }
            }
        }
    }
    else
    {
        if(!usetransparency)
        {
            for(int y = 0; y < size.y; y++)
            {
                for(int x = 0; x < size.x; x++)
                {
                    uint8_t v = *(texture_buffer + start + size.x*y + x);
                    vec2<int> p = RotatePoint({x, y}, size/2, rotation) + pos;
                    SetPixelSafe(context, v, p);
                }
            }
        }
        else
        {
            for(int y = 0; y < size.y; y++)
            {
                for(int x = 0; x < size.x; x++)
                {
                    uint8_t v = *(texture_buffer + start + size.x*y + x);
                    if(v != transparencyvalue)
                    {
                        vec2<int> p = RotatePoint({x, y}, size/2, rotation) + pos;
                        SetPixelSafe(context, v, p);
                    }
                }
            }
        }
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
                        {entity.size.x, entity.size.y}, entity.rotation, entity.data[3]);
                    break;
                }
                case SHAPE::EMPTY_CIRCLE:
                {
                    DrawEmptyCircle(context, entity.data[1], {entity.pos.x, entity.pos.y}, 
                        {entity.size.x, entity.size.y}, entity.data[2], entity.data[3]);
                    break;
                }
                case SHAPE::RECTANGLE:
                {
                    DrawRectangle(context, entity.data[1], entity.data[2], {entity.pos.x, entity.pos.y}, 
                        {entity.size.x, entity.size.y}, entity.rotation, entity.data[3]);
                    break;
                }
                case SHAPE::EMPTY_RECTANGLE:
                {
                    DrawEmptyRectangle(context, entity.data[1], {entity.pos.x, entity.pos.y}, {entity.size.x, entity.size.y}, entity.rotation, entity.data[2]);
                    break;
                }
                case SHAPE::TRIANGLE:
                {
                    DrawTriangle(context, entity.data[1], {entity.pos.x, entity.pos.y}, 
                        {entity.size.x, entity.size.y}, 
                        {*(int16_t*)(entity.data + 2), *(int16_t*)(entity.data + 4)}, 
                        {*(int16_t*)(entity.data + 6), *(int16_t*)(entity.data + 8)}, 
                        {*(int16_t*)(entity.data + 10), *(int16_t*)(entity.data + 12)}, 
                        entity.data[14], entity.rotation);
                    break;
                }
                case SHAPE::MULTI_TRIANGLE:
                {   
                    uint8_t color = entity.data[0];
                    uint16_t start = *(uint16_t*)(entity.data + 1);
                    uint16_t end = *(uint16_t*)(entity.data + 3);
                    if(end < start)
                    {
                        printf("Geometry buffer start (%i) greater than end (%i)\n", (int)start, (int)end);
                        break;
                    }
                    if(end >= GEOMETRY_BUFFER_SIZE)
                    {
                        printf("Geometry buffer end out of range (%i)\n", (int)end);
                        break;
                    }
                    if(start > GEOMETRY_BUFFER_SIZE-1)
                    {
                        printf("Geometry buffer start out of range (%i)\n", (int)start);
                        break;
                    }
                    if((end-start + 1) % 3 != 0)
                    {
                        printf("MULTI_TRIANGLE must be drawn with a number of points divisible by 3, got %i\n", (int)(end-start+1));
                    }
                    vec2<int> center;
                    if(entity.rotation != 0)
                    {
                        for(int i = start; i <= end; i++)
                        {
                            center += geometry_buffer[i];
                        }
                        center /= int(end-start+1);
                    }
                    for(int i = start; i <= end; i+=3)
                    {
                        vec2<int> p0 = geometry_buffer[i];
                        vec2<int> p1 = geometry_buffer[i+1];
                        vec2<int> p2 = geometry_buffer[i+2];
                        if(entity.rotation != 0)
                        {
                            p0 = RotatePoint(p0, center, entity.rotation);
                            p1 = RotatePoint(p1, center, entity.rotation);
                            p2 = RotatePoint(p2, center, entity.rotation);
                        }
                        DrawTriangle(context, color, {entity.pos.x, entity.pos.y}, {entity.size.x, entity.size.y}, 
                            p0, p1, p2, entity.data[5], 0);
                    }
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
            DrawText(context, entity.pos, (FONT)entity.data[0], *(uint16_t*)(entity.data + 1), *(uint16_t*)(entity.data + 3), (TEXT_ALIGNMENT)*(entity.data + 5), entity.data + 6);
            break;
        }
        case ENTITY_TYPE::POINT:
        {
            SetPixelSafe(context, entity.data[0], {entity.pos.x, entity.pos.y});
            break;
        }
        case ENTITY_TYPE::SPRITE:
        {
            DrawSprite(context, entity.pos.convert<int>(), entity.size.convert<int>(), entity.rotation, 
                *(uint32_t*)entity.data, *(uint32_t*)(entity.data + 4), entity.data[8],
                entity.data[9], entity.data[10]);
            break;
        }
        case ENTITY_TYPE::MULTI_LINE:
        {
            uint8_t color = entity.data[0];
            uint16_t start = *(uint16_t*)(entity.data + 1);
            uint16_t end = *(uint16_t*)(entity.data + 3);
            if(end < start)
            {
                printf("Geometry buffer start (%i) greater than end (%i)\n", (int)start, (int)end);
                break;
            }
            if(end >= GEOMETRY_BUFFER_SIZE)
            {
                printf("Geometry buffer end out of range (%i)\n", (int)end);
                break;
            }
            if(start > GEOMETRY_BUFFER_SIZE-1)
            {
                printf("Geometry buffer start out of range (%i)\n", (int)start);
                break;
            }
            if(((end-start)+1) < 2)
            {
                printf("Multi line needs at least 2 points\n");
                break;
            }
            vec2<int> center = {0, 0};
            if(entity.rotation != 0)
            {
                for(int i = start; i <= end; i++)
                {
                    center += geometry_buffer[i];
                }
                center /= int(end-start+1);
            }
            for(int i = start; i <= end-1; i++)
            {
                vec2<int> offset = vec2<int>{(int)entity.pos.x, (int)entity.pos.y};
                vec2<int> p0 = geometry_buffer[i];
                vec2<int> p1 = geometry_buffer[i+1];
                if(entity.rotation != 0)
                {
                    p0 = RotatePoint(p0, center, entity.rotation);
                    p1 = RotatePoint(p1, center, entity.rotation);
                }
                DrawLine(context, color, p0+offset, p1+offset, 0);
            }
            break;
        }
        case ENTITY_TYPE::MULTI_LINES:
        {
            uint8_t color = entity.data[0];
            uint16_t start = *(uint16_t*)(entity.data + 1);
            uint16_t end = *(uint16_t*)(entity.data + 3);
            if(end < start)
            {
                printf("Geometry buffer start (%i) greater than end (%i)\n", (int)start, (int)end);
                break;
            }
            if(end >= GEOMETRY_BUFFER_SIZE)
            {
                printf("Geometry buffer end out of range (%i)\n", (int)end);
                break;
            }
            if(start > GEOMETRY_BUFFER_SIZE-1)
            {
                printf("Geometry buffer start out of range (%i)\n", (int)start);
                break;
            }
            if(((end-start)+1) < 2)
            {
                printf("Multi lines needs at least 2 points\n");
                break;
            }
            if(((end-start)+1) % 2 != 0)
            {
                printf("Multi lines geometry buffer length must be divisible by 2\n");
                break;
            }
            vec2<int> center = {0, 0};
            if(entity.rotation != 0)
            {
                for(int i = start; i <= end; i++)
                {
                    center += geometry_buffer[i];
                }
                center /= int(end-start+1);
            }
            for(int i = start; i <= end-1; i+=2)
            {
                vec2<int> offset = vec2<int>{(int)entity.pos.x, (int)entity.pos.y};
                vec2<int> p0 = geometry_buffer[i];
                vec2<int> p1 = geometry_buffer[i+1];
                if(entity.rotation != 0)
                {
                    p0 = RotatePoint(p0, center, entity.rotation);
                    p1 = RotatePoint(p1, center, entity.rotation);
                }
                DrawLine(context, color, p0+offset, p1+offset, 0);
            }
            break;
        }
        case ENTITY_TYPE::MULTI_POINT:
        {
            uint8_t color = entity.data[0];
            uint16_t start = *(uint16_t*)(entity.data + 1);
            uint16_t end = *(uint16_t*)(entity.data + 3);
            if(end < start)
            {
                printf("Geometry buffer start (%i) greater than end (%i)\n", (int)start, (int)end);
                break;
            }
            if(end >= GEOMETRY_BUFFER_SIZE)
            {
                printf("Geometry buffer end out of range (%i)\n", (int)end);
                break;
            }
            if(start > GEOMETRY_BUFFER_SIZE-2)
            {
                printf("Geometry buffer start out of range (%i)\n", (int)start);
                break;
            }
            vec2<int> center = {0, 0};
            if(entity.rotation != 0)
            {
                for(int i = start; i <= end; i++)
                {
                    center += geometry_buffer[i];
                }
                center /= end-start+1;
            }
            for(int i = start; i <= end; i++)
            {
                vec2<int> p = geometry_buffer[i];
                if(entity.rotation != 0)
                {
                    p = RotatePoint(p, center, entity.rotation);
                }
                p.x += entity.pos.x;
                p.y += entity.pos.y;
                SetPixelSafe(context, color, geometry_buffer[i]+p);
            }
            break;
        }
        case ENTITY_TYPE::BEZIER:
        {
            uint8_t color = entity.data[0];
            uint16_t start = *(uint16_t*)(entity.data + 1);
            uint16_t end = *(uint16_t*)(entity.data + 3);
            if(end < start)
            {
                printf("Geometry buffer start (%i) greater than end (%i)\n", (int)start, (int)end);
                break;
            }
            if(end > GEOMETRY_BUFFER_SIZE)
            {
                printf("Geometry buffer end out of range (%i)\n", (int)end);
                break;
            }
            if(start > GEOMETRY_BUFFER_SIZE-1)
            {
                printf("Geometry buffer start out of range (%i)\n", (int)start);
                break;
            }
            if(end-start <= 1)
            {
                printf("Need at least 2 points for a bezier curve\n");
                break;
            }
            if((end-start+1) % 2 == 0)
            {  
                printf("Need at uneven number of points for a bezier curve, got %i\n", (int)(end-start));
                break;
            }   
            DrawBezier(context, vec2<int>{(int)entity.pos.x, (int)entity.pos.y}, color, geometry_buffer + start, end-start+1, entity.rotation);
            break;
        }

        default:
        {
            printf("Invalid entity type!: %i\n", (int)entity.type);
        }
    }
}