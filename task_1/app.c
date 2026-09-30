#include "sim.h"

/*
    https://ru.wikipedia.org/wiki/Уравнения_мелкой_воды

    Сделал моделирование воды, чем ниже высота, тем темнее пиксель.
    Старовое положение -- это просто 3 волны расположенные в вершинах треугольника.
*/

#define NX 512
#define NY 256
#define STEPS 20000
#define DT 0.2f
#define DX 1.0f
#define G 1.0f
#define H0 1.0f
#define BUMP 20
#define TRI_R 60

void app(void) {
    float h[2][NY][NX];
    float hu[2][NY][NX];
    float hv[2][NY][NX];
    int nx2 = NX / 2;
    int ny2 = NY / 2;
    // vertices of an equilateral triangle centered at (nx2, ny2):
    // top, bottom-left, bottom-right
    int triX[3] = {nx2, nx2 - (int)(0.866f * TRI_R),
                   nx2 + (int)(0.866f * TRI_R)};
    int triY[3] = {ny2 - TRI_R, ny2 + TRI_R / 2, ny2 + TRI_R / 2};
    for (int j = 0; j < NY; ++j)
        for (int i = 0; i < NX; ++i) {
            float bump = 0.0f;
            for (int b = 0; b < 3; ++b) {
                int dx = i - triX[b];
                int dy = j - triY[b];
                if (dx * dx + dy * dy < BUMP * BUMP) {
                    float add = 0.5f * (1.0f - (dx * dx + dy * dy) /
                                                   (float)(BUMP * BUMP));
                    if (add > bump)
                        bump = add;
                }
            }
            h[0][j][i] = H0 + bump;
            hu[0][j][i] = 0.0f;
            hv[0][j][i] = 0.0f;
        }

    for (int step = 0; step < STEPS; ++step) {
        int cur = step % 2;
        int nxt = 1 - cur;
        float cfl = DT / (2.0f * DX);
        for (int j = 0; j < NY; ++j) {
            int jm = (j == 0) ? NY - 1 : j - 1;
            int jp = (j == NY - 1) ? 0 : j + 1;
            for (int i = 0; i < NX; ++i) {
                int im = (i == 0) ? NX - 1 : i - 1;
                int ip = (i == NX - 1) ? 0 : i + 1;

                float hm = h[cur][j][im];
                float hp = h[cur][j][ip];
                float hbw = h[cur][jm][i];
                float hup = h[cur][jp][i];

                float fxm =
                    hu[cur][j][im] * hu[cur][j][im] / hm + 0.5f * G * hm * hm;
                float fxp =
                    hu[cur][j][ip] * hu[cur][j][ip] / hp + 0.5f * G * hp * hp;
                float fym = hv[cur][jm][i] * hu[cur][jm][i] / hbw;
                float fyp = hv[cur][jp][i] * hu[cur][jp][i] / hup;

                float gxm = hu[cur][j][im] * hv[cur][j][im] / hm;
                float gxp = hu[cur][j][ip] * hv[cur][j][ip] / hp;
                float gym = hv[cur][jm][i] * hv[cur][jm][i] / hbw +
                            0.5f * G * hbw * hbw;
                float gyp = hv[cur][jp][i] * hv[cur][jp][i] / hup +
                            0.5f * G * hup * hup;

                float h_av = 0.25f * (hm + hp + hbw + hup);
                float hu_av = 0.25f * (hu[cur][j][im] + hu[cur][j][ip] +
                                       hu[cur][jm][i] + hu[cur][jp][i]);
                float hv_av = 0.25f * (hv[cur][j][im] + hv[cur][j][ip] +
                                       hv[cur][jm][i] + hv[cur][jp][i]);

                h[nxt][j][i] = h_av - cfl * ((hu[cur][j][ip] - hu[cur][j][im]) +
                                             (hv[cur][jp][i] - hv[cur][jm][i]));
                hu[nxt][j][i] = hu_av - cfl * ((fxp - fxm) + (fyp - fym));
                hv[nxt][j][i] = hv_av - cfl * ((gxp - gxm) + (gyp - gym));
            }
        }

        float hmin = h[nxt][0][0];
        float hmax = hmin;
        for (int j = 0; j < NY; ++j)
            for (int i = 0; i < NX; ++i) {
                float v = h[nxt][j][i];
                if (v < hmin)
                    hmin = v;
                if (v > hmax)
                    hmax = v;
            }
        float inv = 255.0f / (hmax - hmin + 1e-9f);
        for (int y = 0; y < SIM_Y_SIZE; ++y) {
            int j = y * NY / SIM_Y_SIZE;
            for (int x = 0; x < SIM_X_SIZE; ++x) {
                int i = x * NX / SIM_X_SIZE;
                int b = (int)((h[nxt][j][i] - hmin) * inv);
                if (b < 0)
                    b = 0;
                if (b > 255)
                    b = 255;
                simPutPixel(x, y, 0xFF000000 + b);
            }
        }
        simFlush();
    }
}
