
#include <math.h>

/**
 * Converts an RGB color value to HSL. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes r, g, and b are contained in the set [0, 255] and
 * returns h, s, and l in the set [0, 1].
 *
 * @param   Number  r       The red color value
 * @param   Number  g       The green color value
 * @param   Number  b       The blue color value
 * @return  Array           The HSL representation
 */
void rgb_to_hsl(float *rgb, float *hsl) {
  float r = rgb[0];
  float g = rgb[1];
  float b = rgb[2];

  float max = fmax(fmax(r, g), b);
  float min = fmin(fmin(r, g), b);

  float h, s, l = (max + min) / 2.0;

  if (max == min) {
    h = s = 0; // achromatic
  } else {
    float d = max - min;
    s = l > 0.5 ? d / (2.0 - max - min) : d / (max + min);

    if (max == r)      h = (g - b) / d + (g < b ? 6.0 : 0);
    else if (max == g) h = (b - r) / d + 2.0;
    else if (max == b) h = (r - g) / d + 4.0;

    h /= 6;
  }

  hsl[0] = h;
  hsl[1] = s;
  hsl[2] = l;
}

float hue_to_rgb(float p, float q, float t) {
  if (t < 0.0) t += 1;
  if (t > 1.0) t -= 1;
  if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
  if (t < 1.0/2.0) return q;
  if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
  return p;
}

/**
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  l       The lightness
 * @return  Array           The RGB representation
 */
void hsl_to_rgb(float *hsl, float *rgb) {
  float r, g, b;
  float h = hsl[0];
  float s = hsl[1];
  float l = hsl[2];

  if (s == 0) {
    r = g = b = l; // achromatic
  } else {

    float q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
    float p = 2 * l - q;

    r = hue_to_rgb(p, q, h + 1.0/3.0);
    g = hue_to_rgb(p, q, h);
    b = hue_to_rgb(p, q, h - 1.0/3.0);
  }

  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
}

/**
 * Converts an RGB color value to HSV. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes r, g, and b are contained in the set [0, 255] and
 * returns h, s, and v in the set [0, 1].
 *
 * @param   Number  r       The red color value
 * @param   Number  g       The green color value
 * @param   Number  b       The blue color value
 * @return  Array           The HSV representation
 */
/* function rgbToHsv(r, g, b) { */
/*   r /= 255, g /= 255, b /= 255; */

/*   var max = Math.max(r, g, b), min = Math.min(r, g, b); */
/*   var h, s, v = max; */

/*   var d = max - min; */
/*   s = max == 0 ? 0 : d / max; */

/*   if (max == min) { */
/*     h = 0; // achromatic */
/*   } else { */
/*     switch (max) { */
/*       case r: h = (g - b) / d + (g < b ? 6 : 0); break; */
/*       case g: h = (b - r) / d + 2; break; */
/*       case b: h = (r - g) / d + 4; break; */
/*     } */

/*     h /= 6; */
/*   } */

/*   return [ h, s, v ]; */
/* } */

/**
 * Converts an HSV color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes h, s, and v are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  v       The value
 * @return  Array           The RGB representation
 */
/* function hsvToRgb(h, s, v) { */
/*   var r, g, b; */

/*   var i = Math.floor(h * 6); */
/*   var f = h * 6 - i; */
/*   var p = v * (1 - s); */
/*   var q = v * (1 - f * s); */
/*   var t = v * (1 - (1 - f) * s); */

/*   switch (i % 6) { */
/*     case 0: r = v, g = t, b = p; break; */
/*     case 1: r = q, g = v, b = p; break; */
/*     case 2: r = p, g = v, b = t; break; */
/*     case 3: r = p, g = q, b = v; break; */
/*     case 4: r = t, g = p, b = v; break; */
/*     case 5: r = v, g = p, b = q; break; */
/*   } */

/*   return [ r * 255, g * 255, b * 255 ]; */
/* } */
