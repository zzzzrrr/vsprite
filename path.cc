#include "path.h"

Path::~Path() {
  loops.clear();
  delete loop;
  delete transform;
  delete gradient;
}

void Path::dump() {
  int n=0;
  Vec2lol::iterator i = loops.begin();
  while(i != loops.end()) {
    printf("LOOP%d= \n", ++n);
    Vec2list::iterator j = (*i)->begin();
    while(j != (*i)->end()) {
      printf(" %f,%f\n", j->x, j->y);
      ++j;
    }
    printf("\n");
    ++i;
  }
}

//------------------------------------------------------------------------
void Path::render() {
#if 0
  //
  // Simple version - just draw the outline
  //
  printf("Rendering");
  Vec2lol::iterator i = loops.begin();
  while(i != loops.end()) {
    glBegin(GL_LINE_STRIP);
    Vec2list::iterator j = (*i)->begin();
    while(j != (*i)->end()) {
      glVertex2f(j->x, j->y);
      ++j;
      printf(".");
    }
    glEnd();
    ++i;
  }
  printf("\n");

#else

  //
  // Fancy stencil buffer method
  // Draws filled concave polygons without tesselation
  //
  // Incompatibilities with certain OpenGL features:
  //   - GL_CULL_FACE interferes with even-odd rule, causing incorrect shapes.
  //   - GL_DEPTH_TEST prevents (some/all) fills from rendering.
  //
  // References:
  //    "Drawing Filled, Concave Polygons Using the Stencil Buffer"
  //    OpenGL Red Book, Chapter 14
  //    http://glprogramming.com/red/chapter14.html#name13
  //
  //    "Hardware accelerated polygon rendering", Zack Rusin, 2006.
  //    http://zrusin.blogspot.com/2006/07/hardware-accelerated-polygon-rendering.html
  //

  // loop
  Vec2lol::iterator i = loops.begin();
  // vertex
  Vec2list::iterator v;

  glPushAttrib(GL_ENABLE_BIT);

  glEnable(GL_STENCIL_TEST);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);

  // Compute a bounding box (for drawing a filled quad behind the stencil)
  float x1,y1,x2,y2;
  v = (*i)->begin();
  x1 = x2= v->x;
  y1 = y2= v->y;
  while(v != (*i)->end()) {
    if(v->x < x1) x1=v->x;
    if(v->y < y1) y1=v->y;
    if(v->x > x2) x2=v->x;
    if(v->y > y2) y2=v->y;
    ++v;
  }

  // Draw to stencil, using the even-odd rule for concave polygons
  glDisable(GL_BLEND);
  glStencilMask(0x01);
  glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);  // INVERT = even-odd rule
  glStencilFunc(GL_ALWAYS, 0, ~0);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  while(i != loops.end()) {
    glBegin(GL_TRIANGLE_FAN);
      v = (*i)->begin();
      while(v != (*i)->end()) {
        glVertex2f(v->x, v->y);
        ++v;
      }
    glEnd ();
    ++i;
  }

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  // Antialiasing: Draw aliased off-pixels to real
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glStencilFunc(GL_EQUAL, 0x00, 0x01);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

  // Draw outline if necessary
  if(stroke[3] != 0) {
    i = loops.begin();
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2);
    while(i != loops.end()) {
      glBegin(GL_LINE_LOOP);
      glColor4ubv(stroke);
      v = (*i)->begin();
      while(v != (*i)->end()) {
        glVertex2f(v->x, v->y);
        ++v;
      }
      glEnd();
      ++i;
    }
    glDisable(GL_LINE_SMOOTH);
  }

  // Draw fill
  glStencilFunc(GL_EQUAL, 0x01, 0x01);
  glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

  // Draw a filled quad behind the stencil
  glBegin(GL_QUADS);
    GLubyte *color;
    color = gradient ? gradient->interp(b2Vec2(x1,y1)) : fill;
    glColor4ubv(color);
    glVertex2f(x1,y1);
    color = gradient ? gradient->interp(b2Vec2(x2,y1)) : fill;
    glColor4ubv(color);
    glVertex2f(x2,y1);
    color = gradient ? gradient->interp(b2Vec2(x2,y2)) : fill;
    glColor4ubv(color);
    glVertex2f(x2,y2);
    color = gradient ? gradient->interp(b2Vec2(x1,y2)) : fill;
    glColor4ubv(color);
    glVertex2f(x1,y2);
  glEnd();

  glPopAttrib();
#endif
}
