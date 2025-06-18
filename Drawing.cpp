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
void DrawRectangle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> size, uint8_t rotation)
{
    if(rotation == 0)
    {
        size.x -= 1;
        size.y -= 1;
        pos.x += 1;
        pos.y += 1;
        for(int i = std::max(0, pos.x-size.x/2); i < std::min(context.screen_size.x, pos.x+size.x/2); i++)
        {
            for(int ii = std::max(0, pos.y-size.y/2); ii < std::min(context.screen_size.y, pos.y+size.y/2); ii++)
            {
                SetPixel(context, fill, {i, ii});
            }
        }
        size.x += 1;
        size.y += 1;
        pos.x -= 1;
        pos.y -= 1;
        for(int i = 0; i < size.x; i++)
            SetPixel(context, border, {i+pos.x-size.x/2, pos.y-size.y/2});
        for(int i = 0; i < size.x; i++)
            SetPixel(context, border, {i+pos.x-size.x/2, pos.y+size.y/2});
        for(int i = 0; i < size.y; i++)
            SetPixel(context, border, {pos.x-size.x/2, i+pos.y-size.y/2});
        for(int i = 0; i < size.y; i++)
            SetPixel(context, border, {pos.x+size.x/2, i+pos.y-size.y/2});
    }
    else
    {
        
    }
}

int TriangleArea(int x1, int y1, int x2, int y2, int x3, int y3)
{
   return abs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))/2);
}

/* Stolen from stackoverflow. A function to check whether point P(x, y) lies inside the triangle formed 
   by A(x1, y1), B(x2, y2) and C(x3, y3) */
bool IsInsideTriangle(int A, int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
{   
   /* Calculate area of triangle PBC */  
   int A1 = TriangleArea (x, y, x2, y2, x3, y3);

   /* Calculate area of triangle PAC */  
   int A2 = TriangleArea (x1, y1, x, y, x3, y3);

   /* Calculate area of triangle PAB */   
   int A3 = TriangleArea (x1, y1, x2, y2, x, y);

   /* Check if sum of A1, A2 and A3 is same as A */
   return (A == A1 + A2 + A3);
}

void DrawTriangle(const ScreenContext& context, uint8_t fill, vec2<int> pos, vec2<int> size, vec2<int> p0, vec2<int> p1, vec2<int> p2, bool centertriangle)
{
    p0 += pos;
    p1 += pos;
    p2 += pos;
    vec2<int> center = (p0+p1+p2)/vec2<int>{3, 3};
    p0 = ((p0-center)*size) + center;
    p1 = ((p1-center)*size) + center;
    p2 = ((p2-center)*size) + center;
    vec2<int> topl = {std::min({p0.x, p1.x, p0.x}), std::min({p0.y, p1.y, p2.y})};
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

    /* Calculate area of triangle ABC */
    float A = TriangleArea (p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
    for(int y = std::max(0, topl.y); y < std::min(botr.y, context.screen_size.y); y++)
    {
        for(int x = std::max(0, topl.x); x < std::min(botr.x, context.screen_size.x); x++)
        {
            if(IsInsideTriangle(A, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, x, y))
            {
                SetPixel(context, fill, {x, y});
            }
        }
    }
}
void DrawLine(const ScreenContext& context, uint8_t fill, vec2<int> p0, vec2<int> p1)
{
    p0 = p0.min(context.screen_size).max(0);
    p1 = p1.min(context.screen_size).max(0);
    vec2<int> dif = p1-p0;
    int len = sqrt(abs(dif.x*dif.y));

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
                case SHAPE::RECTANGLE:
                {
                    DrawRectangle(context, entity.data[1], entity.data[2], {entity.pos.x, entity.pos.y}, 
                        {entity.size.x, entity.size.y}, entity.rotation);
                    break;
                }
                case SHAPE::TRIANGLE:
                {
                    DrawTriangle(context, entity.data[1], {entity.pos.x, entity.pos.y}, 
                        {entity.size.x, entity.size.y}, {entity.data[2], entity.data[3]}, 
                        {entity.data[4], entity.data[5]}, {entity.data[6], entity.data[7]}, entity.data[8]);
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
            DrawLine(context, entity.data[0], {entity.pos.x, entity.pos.y}, {entity.size.x, entity.size.y});
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