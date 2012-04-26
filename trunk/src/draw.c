#include "draw.h"
#include "graphics.h"

#include <string.h>

gltexture_t gltextures[MAX_GLTEXTURES];
uint16_t    numgltextures;
GLuint      currenttexture;

void gl_draw_init(void) {
    currenttexture = -1;
    numgltextures = 0;
}

void gl_draw_finish(void) {
    for(unsigned i = 0; i < numgltextures; ++i) {
        glDeleteTextures(1, &gltextures[i].texnum);
    }
}

void gl_bind(GLuint texnum) {
    if(currenttexture == texnum)
        return;
    currenttexture = texnum;
    glBindTexture(GL_TEXTURE_2D, texnum);
}

gltexture_t* gl_find_gltexture(const char* ident) {
    for(uint16_t i = 0; i < numgltextures; ++i) {
        if(!strcmp(ident, gltextures[i].identifier))
            return &gltextures[i];
    }
    return NULL;
}

GLuint gl_find_texture(const char* ident) {
    for(uint16_t i = 0; i < numgltextures; ++i) {
        if(!strcmp(ident, gltextures[i].identifier))
            return gltextures[i].texnum;
    }
    return 0;
}

GLuint gl_load_texture(const char* ident, int width, int height, byte* data) {
    GLuint texture = 0;
    gltexture_t* tex = NULL;

    //Already loaded?
    if(ident[0] && (texture = gl_find_texture(ident)) > 0) {
        return texture;
    }

    if(data == NULL) {
        error("gl_load_texture() : data is NULL");
    }

    glGenTextures(1, &texture);
    if(!texture) {
        error("gl_load_texture() : glGenTextures() failure: %d", glGetError());
    }
    tex = &gltextures[numgltextures];
    numgltextures++;

    glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    tex->height = height;
    tex->width = width;
    tex->texnum = texture;
    strcpy(tex->identifier, ident);

    return texture;
}

///Loads a TGA file into texture cache.
GLuint gl_load_from_file(const char* filePath, const char* ident) {
    static const byte tga_cmp[12] = {0,0,2,0,0,0,0,0,0,0,0,0};

    byte* data  = NULL;
    FILE* fp    = NULL;
    byte tga_header[12];
    byte header[6];
    GLuint texid = -1;


    fp = fopen(filePath, "rb");
    if(!fp) return -1;

    //Read the header, compare it against the valid header, then read the juicy information.
    if(fread(&tga_header, 1, sizeof(tga_header), fp) != sizeof(tga_header) ||
       memcmp(tga_header, tga_cmp, sizeof(tga_header)) ||
       fread(&header, 1, sizeof(header), fp) != sizeof(header))
    {
        error("gl_load_from_file: Cannot open '%s'. Unknown format.", filePath);
        if(fp) fclose(fp);
        return -1;
    }

    unsigned w = header[1] * 256 + header[0], h = header[3] * 256 + header[2];
    if(w <= 0 || h <= 0 || (header[4] != 24 && header[4] != 32)) {
        error("gl_load_from_file: Invalid header. w: %d, h: %d, bpp: %d.", w, h, header[4]);
        if(fp) fclose(fp);
        return -1;
    }

    uint img_size = w*h*(header[4]>>3);
    data = (byte*)malloc(img_size);

    if(!data) {
        if(fp) fclose(fp);
        error("Out of memory.");
    }

    if(fread(data, 1, img_size, fp) != img_size) {
        if(fp) fclose(fp);
        if(data) free(data);
        error("gl_load_from_file: Image size does not match header specifications.");
    }

    for(uint i = 0; i < img_size; i+=(header[4]>>3)) {
        data[i] ^= data[i + 2];
        data[i + 2] ^= data[i];
        data[i] ^= data[i + 2];
    }

    fclose(fp);

    //load the texture
    texid = gl_load_texture(ident, w, h, data);

    //free the data
    free(data);

    return texid;
}

void gl_draw_string_spritesheet(gltexture_t* spritesheet, const char* str, int x, int y ) {
    char* s = (char*)str;
    while(*s) {
        if(*s >= '!' && *s <= '~')
            gl_draw_tile_spritesheet(spritesheet, TILE_WIDTH*(*s), x, y);
        x += TILE_WIDTH; s++;
    }
}

void gl_draw_tile_spritesheet(gltexture_t* spritesheet, unsigned tile_x, int x, int y ) {
    #ifdef _DEBUG // just to check and make sure we have our animiations right, only temporary
        if(tile_x > spritesheet->width) {
            warn("tile_x is greater than width. %d > %d", tile_x, spritesheet->width);
        }
    #endif

    gl_bind(spritesheet->texnum);
    glBegin(GL_QUADS);
        glTexCoord2d(tile_x/(float)spritesheet->width, 0);              glVertex2f(x,y);
        glTexCoord2d((tile_x+TILE_WIDTH)/(float)spritesheet->width,0);  glVertex2f(x+TILE_WIDTH,y);
        glTexCoord2d((tile_x+TILE_WIDTH)/(float)spritesheet->width,1);  glVertex2f(x+TILE_WIDTH,y+TILE_HEIGHT);
        glTexCoord2d(tile_x/(float)spritesheet->width, 1);              glVertex2f(x,y+TILE_HEIGHT);
    glEnd();
}

void gl_draw_image(int x, int y, unsigned w, unsigned h, GLuint texture) {
    gl_bind(texture);
    glBegin( GL_QUADS );
        glTexCoord2f(0, 0); glVertex3f(x, y, 0);
        glTexCoord2f(1, 0); glVertex3f(x+w, y , 0);
        glTexCoord2f(1, 1); glVertex3f(x+w, y+h, 0);
        glTexCoord2f(0, 1); glVertex3f(x, y+h, 0);
    glEnd();
}

void gl_draw_tile(int x, int y, GLuint texture) {
    gl_draw_image(x,y,TILE_WIDTH,TILE_HEIGHT,texture);
}
