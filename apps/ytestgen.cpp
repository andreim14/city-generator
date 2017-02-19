//
// LICENSE:
//
// Copyright (c) 2016 -- 2017 Fabio Pellacini
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

// general includes ------------
#include "yapp.h"

#include <map>
#include <set>

#include "../yocto/yocto_cmd.h"
#include "../yocto/yocto_math.h"
#include "../yocto/yocto_obj.h"
#include "../yocto/yocto_shape.h"

#include "sunsky/ArHosekSkyModel.c"
#include "sunsky/ArHosekSkyModel.h"

template <typename T>
std::vector<T>& operator+=(std::vector<T>& a, const std::vector<T>& b) {
    for (auto aa : b) a.push_back(aa);
    return a;
}

template <typename T>
std::vector<T>& operator+=(std::vector<T>& a, const T& b) {
    a.push_back(b);
    return a;
}

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b) {
    auto c = a;
    c += b;
    return c;
}

template <typename T>
std::vector<T> operator+(const T& a, const std::vector<T>& b) {
    auto c = std::vector<T*>{a};
    c += b;
    return c;
}

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const T& b) {
    auto c = a;
    c += b;
    return c;
}

ym::frame3f make_frame(const ym::vec3f& pos,
                       const ym::vec3f& rot = ym::zero3f) {
    ym::frame3f xf = ym::identity_frame3f;
    xf = rotation_frame3(ym::vec3f{1, 0, 0}, rot[0] * ym::pif / 180) * xf;
    xf = rotation_frame3(ym::vec3f{0, 1, 0}, rot[1] * ym::pif / 180) * xf;
    xf = rotation_frame3(ym::vec3f{0, 0, 1}, rot[2] * ym::pif / 180) * xf;
    xf = translation_frame3(pos) * xf;
    return xf;
}

ym::frame3f make_lookat_frame(const ym::vec3f& pos, const ym::vec3f& to) {
    auto xf = lookat_frame3(pos, to, {0, 1, 0});
    xf[2] = -xf[2];
    xf[0] = -xf[0];
    return xf;
}

std::shared_ptr<yapp::texture> make_texture(const std::string& path) {
    auto txt = std::make_shared<yapp::texture>();
    txt->path = path;
    return txt;
}

std::shared_ptr<yapp::material> make_material(
    const std::string& name, const ym::vec3f& ke, const ym::vec3f& kd,
    const ym::vec3f& ks, const ym::vec3f& kt, float rs,
    std::shared_ptr<yapp::texture> ke_txt,
    std::shared_ptr<yapp::texture> kd_txt,
    std::shared_ptr<yapp::texture> ks_txt,
    std::shared_ptr<yapp::texture> kt_txt) {
    auto mat = std::make_shared<yapp::material>();
    mat->name = name;
    mat->ke = ke;
    mat->kd = kd;
    mat->ks = ks;
    mat->kt = kt;
    mat->rs = rs;
    mat->ke_txt = ke_txt;
    mat->kd_txt = kd_txt;
    mat->ks_txt = ks_txt;
    mat->kt_txt = kt_txt;
    return mat;
}

std::shared_ptr<yapp::material> make_emission(
    const std::string& name, const ym::vec3f& ke,
    std::shared_ptr<yapp::texture> txt = nullptr) {
    return make_material(name, ke, ym::zero3f, ym::zero3f, ym::zero3f, 0, txt,
                         nullptr, nullptr, nullptr);
}

std::shared_ptr<yapp::material> make_diffuse(
    const std::string& name, const ym::vec3f& kd,
    std::shared_ptr<yapp::texture> txt = nullptr) {
    return make_material(name, ym::zero3f, kd, ym::zero3f, ym::zero3f, 0,
                         nullptr, txt, nullptr, nullptr);
}

std::shared_ptr<yapp::material> make_plastic(
    const std::string& name, const ym::vec3f& kd, float rs,
    std::shared_ptr<yapp::texture> txt = nullptr) {
    return make_material(name, ym::zero3f, kd, {0.04f, 0.04f, 0.04f},
                         ym::zero3f, rs, nullptr, txt, nullptr, nullptr);
}

std::shared_ptr<yapp::material> make_metal(
    const std::string& name, const ym::vec3f& kd, float rs,
    std::shared_ptr<yapp::texture> txt = nullptr) {
    return make_material(name, ym::zero3f, ym::zero3f, kd, ym::zero3f, rs,
                         nullptr, nullptr, txt, nullptr);
}

std::shared_ptr<yapp::material> make_glass(
    const std::string& name, const ym::vec3f& kd, float rs,
    std::shared_ptr<yapp::texture> txt = nullptr) {
    return make_material(name, ym::zero3f, ym::zero3f, {0.04f, 0.04f, 0.04f},
                         kd, rs, nullptr, nullptr, txt, nullptr);
}

enum struct mtype {
    def,
    matte,
    matte_txt,
    floor,
    floor_txt,
    plastic00,
    plastic01,
    plastic02,
    plastic03,
    plastic00_txt,
    plastic01_txt,
    plastic02_txt,
    plastic03_txt,
    metal00,
    metal01,
    metal02,
    metal03,
    glass00,
    glass01,
    glass02,
    glass03,
    lines00,
    lines01,
    lines02,
    lines03,
    lines01_txt,
    lines02_txt,
    lines03_txt,
    points00,
    points01,
    points01_txt,
    pointlight01,
    pointlight02,
    arealight01,
    arealight02,
    env00,
    env01
};

std::shared_ptr<yapp::material> make_material(mtype type) {
    switch (type) {
        case mtype::def:
        case mtype::matte: return make_diffuse("gray", {0.2f, 0.2f, 0.2f});
        case mtype::floor: return make_diffuse("floor", {0.2f, 0.2f, 0.2f});
        case mtype::matte_txt:
            return make_diffuse("gray_txt", {1, 1, 1},
                                make_texture("grid.png"));
        case mtype::floor_txt:
            return make_diffuse("floor_txt", {1, 1, 1},
                                make_texture("grid.png"));
        case mtype::plastic00:
            return make_plastic("plastic00", {0.2f, 0.2f, 0.2f}, 0.01f);
        case mtype::plastic01:
            return make_plastic("plastic01", {0.5f, 0.2f, 0.2f}, 0.1f);
        case mtype::plastic02:
            return make_plastic("plastic02", {0.2f, 0.5f, 0.2f}, 0.05f);
        case mtype::plastic03:
            return make_plastic("plastic03", {0.2f, 0.2f, 0.5f}, 0.01f);
        case mtype::plastic00_txt:
            return make_plastic("plastic01_txt", {1, 1, 1}, 0.1f,
                                make_texture("grid.png"));
        case mtype::plastic01_txt:
            return make_plastic("plastic01_txt", {1, 1, 1}, 0.1f,
                                make_texture("rcolored.png"));
        case mtype::plastic02_txt:
            return make_plastic("plastic02_txt", {1, 1, 1}, 0.05f,
                                make_texture("checker.png"));
        case mtype::plastic03_txt:
            return make_plastic("plastic03_txt", {1, 1, 1}, 0.01f,
                                make_texture("colored.png"));
        case mtype::metal00:
            return make_metal("metal00", {0.8f, 0.8f, 0.8f}, 0);
        case mtype::metal01:
            return make_metal("metal01", {0.8f, 0.8f, 0.8f}, 0);
        case mtype::metal02:
            return make_metal("metal02", {0.8f, 0.8f, 0.8f}, 0.01f);
        case mtype::metal03:
            return make_metal("metal03", {0.8f, 0.8f, 0.8f}, 0.05f);
        case mtype::glass00:
            return make_glass("glass00", {0.8f, 0.8f, 0.8f}, 0);
        case mtype::glass01:
            return make_glass("glass01", {0.8f, 0.8f, 0.8f}, 0);
        case mtype::glass02:
            return make_glass("glass02", {0.8f, 0.8f, 0.8f}, 0.01f);
        case mtype::glass03:
            return make_glass("glass03", {0.8f, 0.8f, 0.8f}, 0.05f);
        case mtype::lines00: return make_diffuse("lines00", {0.2f, 0.2f, 0.2f});
        case mtype::lines01: return make_diffuse("lines01", {0.2f, 0.2f, 0.2f});
        case mtype::lines02: return make_diffuse("lines02", {0.2f, 0.2f, 0.2f});
        case mtype::lines03: return make_diffuse("lines03", {0.2f, 0.2f, 0.2f});
        case mtype::lines01_txt:
            return make_diffuse("lines01_txt", {0.2f, 0.2f, 0.2f});
        case mtype::lines02_txt:
            return make_diffuse("lines02_txt", {0.2f, 0.2f, 0.2f});
        case mtype::lines03_txt:
            return make_diffuse("lines03_txt", {0.2f, 0.2f, 0.2f});
        case mtype::points00:
            return make_diffuse("points00", {0.2f, 0.2f, 0.2f});
        case mtype::points01:
            return make_diffuse("points01", {0.2f, 0.2f, 0.2f});
        case mtype::points01_txt:
            return make_diffuse("points01_txt", {0.2f, 0.2f, 0.2f});
        case mtype::pointlight01:
            return make_emission("pointlight01", {100, 100, 100});
        case mtype::pointlight02:
            return make_emission("pointlight02", {100, 100, 100});
        case mtype::arealight01:
            return make_emission("arealight01", {40, 40, 40});
        case mtype::arealight02:
            return make_emission("arealight02", {40, 40, 40});
        case mtype::env00: return make_emission("env00", {1, 1, 1});
        case mtype::env01:
            return make_emission("env01", {1, 1, 1}, make_texture("env.hdr"));
    }
}

std::shared_ptr<yapp::shape> make_shape(
    const std::string& name, std::shared_ptr<yapp::material> mat, int l,
    yshape::stdsurface_type stype,
    const ym::frame3f& frame = ym::identity_frame3f,
    const ym::vec3f& scale = {1, 1, 1}) {
    ym::vec4f params = {0.75f, 0.75f, 0, 0};
    auto shape = std::make_shared<yapp::shape>();
    shape->name = name;
    shape->mat = mat;
    yshape::make_stdsurface(stype, l, params, shape->triangles, shape->pos,
                            shape->norm, shape->texcoord);
    for (auto& p : shape->pos) (ym::vec3f&)p *= scale;
    shape->frame = frame;
    return shape;
}

std::shared_ptr<yapp::shape> make_shape(
    const std::string& name, mtype mtype, int l, yshape::stdsurface_type stype,
    const ym::frame3f& frame = ym::identity_frame3f,
    const ym::vec3f& scale = {1, 1, 1}) {
    return make_shape(name, make_material(mtype), l, stype, frame, scale);
}

std::shared_ptr<yapp::shape> _make_floor(
    const std::string& name, std::shared_ptr<yapp::material> mat, float s = 6,
    float p = 4, int l = 6, const ym::frame3f& frame = make_frame({0, 0, -4}),
    const ym::vec3f& scale = {6, 6, 6}) {
    auto n = (int)round(powf(2, (float)l));
    auto shape = std::make_shared<yapp::shape>();
    shape->name = name;
    shape->mat = mat;
    yshape::make_uvsurface(n, n, shape->triangles, shape->pos, shape->norm,
                           shape->texcoord,
                           [p, scale](const ym::vec2f& uv) {
                               auto pos = ym::zero3f;
                               auto x = 2 * uv[0] - 1;
                               auto y = 2 * (1 - uv[1]) - 1;
                               if (y >= 0 || !p) {
                                   pos = {x, 0, y};
                               } else {
                                   pos = {x, std::pow(-y, p), y};
                               }
                               return scale * pos;
                           },
                           [](const ym::vec2f& uv) {
                               return ym::vec3f{0, 1, 0};
                           },
                           [s](const ym::vec2f& uv) { return uv * s; });
    if (p) {
        yshape::compute_normals((int)shape->points.size(), shape->points.data(),
                                (int)shape->lines.size(), shape->lines.data(),
                                (int)shape->triangles.size(),
                                shape->triangles.data(), (int)shape->pos.size(),
                                shape->pos.data(), shape->norm.data());
    }
    shape->frame = frame;
    return shape;
}

std::shared_ptr<yapp::shape> make_lines(const std::string& name,
                                        std::shared_ptr<yapp::material> mat,
                                        int num, int n, float r, float c,
                                        float s, const ym::frame3f& frame,
                                        const ym::vec3f& scale) {
    auto shape = std::make_shared<yapp::shape>();
    shape->name = name;
    shape->mat = mat;

    ym::rng_pcg32 rn;
    std::vector<ym::vec3f> base(num + 1), dir(num + 1);
    std::vector<float> ln(num + 1);
    for (auto i = 0; i <= num; i++) {
        auto z = -1 + 2 * next1f(&rn);
        auto r = std::sqrt(ym::clamp(1 - z * z, (float)0, (float)1));
        auto phi = 2 * ym::pif * next1f(&rn);
        base[i] = ym::vec3f{r * std::cos(phi), r * std::sin(phi), z};
        dir[i] = base[i];
        ln[i] = 0.15f + 0.15f * next1f(&rn);
    }

    yshape::make_lines(
        n, num, shape->lines, shape->pos, shape->norm, shape->texcoord,
        shape->radius,
        [num, base, dir, ln, r, s, c, &rn, scale](const ym::vec2f& uv) {
            auto i = ym::clamp((int)(uv[1] * (num + 1)), 0, num);
            auto pos = base[i] * (1 + uv[0] * ln[i]);
            if (r) {
                pos += ym::vec3f{r * (0.5f - next1f(&rn)),
                                 r * (0.5f - next1f(&rn)),
                                 r * (0.5f - next1f(&rn))};
            }
            if (s && uv[0]) {
                ym::frame3f rotation =
                    rotation_frame3(ym::vec3f{0, 1, 0}, s * uv[0] * uv[0]);
                pos = transform_point(rotation, pos);
            }
            auto nc = 128;
            if (c && i > nc) {
                int cc = 0;
                float md = HUGE_VALF;
                for (int k = 0; k < nc; k++) {
                    float d = dist(base[i], base[k]);
                    if (d < md) {
                        md = d;
                        cc = k;
                    }
                }
                ym::vec3f cpos = base[cc] * (1 + uv[0] * ln[cc]);
                pos =
                    pos * (1 - c * uv[0] * uv[0]) + cpos * (c * uv[0] * uv[0]);
            }
            return scale * pos;
        },
        [](const ym::vec2f& uv) {
            return ym::vec3f{0, 0, 1};
        },
        [](const ym::vec2f& uv) { return uv; },
        [](const ym::vec2f& uv) { return 0.001f + 0.001f * (1 - uv[0]); });

    yshape::compute_normals((int)shape->points.size(), shape->points.data(),
                            (int)shape->lines.size(), shape->lines.data(),
                            (int)shape->triangles.size(),
                            shape->triangles.data(), (int)shape->pos.size(),
                            shape->pos.data(), shape->norm.data());
    shape->frame = frame;
    return shape;
}

std::shared_ptr<yapp::shape> make_points(const std::string& name,
                                         std::shared_ptr<yapp::material> mat,
                                         int num, const ym::frame3f& frame,
                                         const ym::vec3f& scale) {
    auto shape = std::make_shared<yapp::shape>();
    shape->name = name;
    shape->mat = mat;

    ym::rng_pcg32 rn;
    yshape::make_points(
        num, shape->points, shape->pos, shape->norm, shape->texcoord,
        shape->radius,
        [&rn, scale](float u) {
            return scale * ym::vec3f{next1f(&rn), next1f(&rn), next1f(&rn)};
        },
        [](float u) {
            return ym::vec3f{0, 0, 1};
        },
        [](float u) {
            return ym::vec2f{u, 0};
        },
        [](float u) { return 0.0025f; });
    shape->frame = frame;
    return shape;
}

std::shared_ptr<yapp::shape> make_point(const std::string& name,
                                        std::shared_ptr<yapp::material> mat,
                                        const ym::frame3f& frame,
                                        float radius = 0.001f) {
    auto shape = std::make_shared<yapp::shape>();
    shape->name = name;
    shape->mat = mat;
    shape->points.push_back(0);
    shape->pos.push_back(frame[3]);
    shape->norm.push_back({0, 0, 1});
    shape->radius.push_back(radius);
    return shape;
}

std::vector<std::shared_ptr<yapp::shape>> make_random_shapes(int nshapes,
                                                             int l);

enum struct stype {
    floor02,
    obj01,
    obj02,
    obj03,
    matball01,
    lines01,
    lines02,
    lines03,
    points01,
    pointlight01,
    arealight01,
    envsphere00,
    envsphere01,
    simple_objs,
    simple_objs_notxt,
    simple_lines,
    simple_points,
    simple_pointlights,
    simple_arealights,
    simple_matballs01,
    simple_matballs02,
    simple_matballs03,
    simple_matballs04,
    simple_random32,
};

std::vector<std::shared_ptr<yapp::shape>> make_shapes(
    stype st, mtype mt = mtype::def,
    const ym::frame3f& frame = ym::identity_frame3f,
    const ym::vec3f& scale = ym::vec3f{1, 1, 1}) {
    switch (st) {
        case stype::floor02: {
            auto mat = (mt == mtype::def) ? make_material(mtype::floor_txt)
                                          : make_material(mt);
            return {_make_floor("floor02_" + mat->name, mat)};
        } break;
        case stype::obj01: {
            auto mat = (mt == mtype::def) ? make_material(mtype::plastic00)
                                          : make_material(mt);
            return {make_shape("simple_obj01_" + mat->name, mat, 5,
                               yshape::stdsurface_type::uvflipcapsphere, frame,
                               ym::vec3f{0.5f, 0.5f, 0.5f} * scale)};
        } break;
        case stype::obj02: {
            auto mat = (mt == mtype::def) ? make_material(mtype::plastic00)
                                          : make_material(mt);
            return {make_shape("simple_obj02_" + mat->name, mat, 4,
                               yshape::stdsurface_type::uvspherizedcube, frame,
                               ym::vec3f{0.5f, 0.5f, 0.5f} * scale)};
        } break;
        case stype::obj03: {
            auto mat = (mt == mtype::def) ? make_material(mtype::plastic00)
                                          : make_material(mt);
            return {make_shape("simple_obj03_" + mat->name, mat, 4,
                               yshape::stdsurface_type::uvspherecube, frame,
                               ym::vec3f{0.5f, 0.5f, 0.5f} * scale)};
        } break;
        case stype::simple_objs: {
            return make_shapes(stype::obj01, mtype::plastic01_txt,
                               make_frame({-1.25f, 0.5f, 0}, scale)) +
                   make_shapes(stype::obj02, mtype::plastic02_txt,
                               make_frame({0, 0.5f, 0}, scale)) +
                   make_shapes(stype::obj03, mtype::plastic03_txt,
                               make_frame({1.25f, 0.5f, 0}, scale));
        } break;
        case stype::simple_objs_notxt: {
            return make_shapes(stype::obj01, mtype::plastic01,
                               make_frame({-1.25f, 0.5f, 0}, scale)) +
                   make_shapes(stype::obj02, mtype::plastic02,
                               make_frame({0, 0.5f, 0}, scale)) +
                   make_shapes(stype::obj03, mtype::plastic03,
                               make_frame({1.25f, 0.5f, 0}, scale));
        } break;
        case stype::lines01: {
            auto mat = (mt == mtype::def) ? make_material(mtype::lines00)
                                          : make_material(mt);
            return {make_lines("simple_lines01_" + mat->name, mat, 64 * 64 * 16,
                               4, 0.1f, 0, 0, frame, {0.5f, 0.5f, 0.5f}),
                    make_shape("simple_interior02_", mtype::matte, 6,
                               yshape::stdsurface_type::uvsphere, frame,
                               ym::vec3f{0.5f, 0.5f, 0.5f} * scale)};
        } break;
        case stype::lines02: {
            auto mat = (mt == mtype::def) ? make_material(mtype::lines00)
                                          : make_material(mt);
            return {make_lines("simple_lines02_" + mat->name, mat, 64 * 64 * 16,
                               4, 0, 0.75f, 0, frame, {0.5f, 0.5f, 0.5f}),
                    make_shape("simple_interior02_", mtype::matte, 6,
                               yshape::stdsurface_type::uvsphere, frame,
                               ym::vec3f{0.5f, 0.5f, 0.5f} * scale)};
        } break;
        case stype::lines03: {
            auto mat = (mt == mtype::def) ? make_material(mtype::lines00)
                                          : make_material(mt);
            return {make_lines("simple_lines03_" + mat->name, mat, 64 * 64 * 16,
                               4, 0, 0, 0.5f, frame, {0.5f, 0.5f, 0.5f}),
                    make_shape("simple_interior02_", mtype::matte, 6,
                               yshape::stdsurface_type::uvsphere, frame,
                               ym::vec3f{0.5f, 0.5f, 0.5f} * scale)};
        } break;
        case stype::simple_lines: {
            return make_shapes(stype::lines01, mtype::lines01,
                               make_frame({-1.25f, 0.5f, 0}), scale) +
                   make_shapes(stype::lines02, mtype::lines02,
                               make_frame({0, 0.5f, 0}), scale) +
                   make_shapes(stype::lines03, mtype::lines03,
                               make_frame({1.25f, 0.5f, 0}), scale);
        } break;
        case stype::points01: {
            auto mat = (mt == mtype::def) ? make_material(mtype::points01)
                                          : make_material(mt);
            return {make_points("points01_" + mat->name, mat, 64 * 64 * 16,
                                frame, ym::vec3f{0.5f, 0.5f, 0.5f} * scale)};
        } break;
        case stype::simple_points: {
            return make_shapes(stype::points01, mtype::points01,
                               make_frame({0, 0.5f, 0}), scale);
        } break;
        case stype::pointlight01: {
            auto mat = (mt == mtype::def) ? make_material(mtype::pointlight01)
                                          : make_material(mt);
            return {make_point("pointlight01_" + mat->name, mat, frame)};
        } break;
        case stype::simple_pointlights: {
            return make_shapes(stype::pointlight01, mtype::pointlight01,
                               make_frame({0.7f, 4, 3}), scale) +
                   make_shapes(stype::pointlight01, mtype::pointlight01,
                               make_frame({-0.7f, 4, 3}), scale);
        } break;
        case stype::arealight01: {
            auto mat = (mt == mtype::def) ? make_material(mtype::arealight01)
                                          : make_material(mt);
            return {make_shape("arealight01_" + mat->name, mat, 0,
                               yshape::stdsurface_type::uvquad, frame,
                               ym::vec3f{0.5f, 0.5f, 0.5f} * scale)};
        } break;
        case stype::simple_arealights: {
            return make_shapes(stype::arealight01, mtype::arealight01,
                               make_lookat_frame({-2, 2, 4}, {0, 1, 0}),
                               ym::vec3f{2, 2, 2} * scale) +
                   make_shapes(stype::arealight01, mtype::arealight01,
                               make_lookat_frame({2, 2, 4}, {0, 1, 0}),
                               ym::vec3f{2, 2, 2} * scale);
        } break;
        case stype::envsphere00: {
            auto mat = (mt == mtype::def) ? make_material(mtype::env00)
                                          : make_material(mt);
            return {make_shape("envsphere00_" + mat->name, mat, 6,
                               yshape::stdsurface_type::uvflippedsphere,
                               make_frame({0, 0.5f, 0}, {-90, 0, 0}),
                               ym::vec3f{10000, 10000, 10000} * scale)};
        } break;
        case stype::envsphere01: {
            auto mat = (mt == mtype::def) ? make_material(mtype::env01)
                                          : make_material(mt);
            return {make_shape("envsphere01_" + mat->name, mat, 6,
                               yshape::stdsurface_type::uvflippedsphere,
                               make_frame({0, 0.5f, 0}, {-90, 0, 0}),
                               ym::vec3f{10000, 10000, 10000} * scale)};
        } break;
        case stype::matball01: {
            auto mat = (mt == mtype::def) ? make_material(mtype::plastic00)
                                          : make_material(mt);
            return {make_shape("intmatball01_" + mat->name,
                               make_material(mtype::matte), 5,
                               yshape::stdsurface_type::uvsphere, frame,
                               ym::vec3f{0.4f, 0.4f, 0.4f} * scale),
                    make_shape("matball01_" + mat->name, mat, 5,
                               yshape::stdsurface_type::uvflipcapsphere,
                               frame * make_frame({0, 0, 0}, {0, 35, 45}),
                               ym::vec3f{0.5f, 0.5f, 0.5f} * scale)};
        } break;
        case stype::simple_matballs01: {
            return make_shapes(stype::matball01, mtype::plastic01,
                               make_frame({-1.25f, 0.5f, 0}), scale) +
                   make_shapes(stype::matball01, mtype::plastic02,
                               make_frame({0, 0.5f, 0}), scale) +
                   make_shapes(stype::matball01, mtype::plastic03,
                               make_frame({1.25f, 0.5f, 0}), scale);
        } break;
        case stype::simple_matballs02: {
            return make_shapes(stype::matball01, mtype::plastic01_txt,
                               make_frame({-1.25f, 0.5f, 0}), scale) +
                   make_shapes(stype::matball01, mtype::plastic02_txt,
                               make_frame({0, 0.5f, 0}), scale) +
                   make_shapes(stype::matball01, mtype::plastic03_txt,
                               make_frame({1.25f, 0.5f, 0}), scale);
        } break;
        case stype::simple_matballs03: {
            return make_shapes(stype::matball01, mtype::metal01,
                               make_frame({-1.25f, 0.5f, 0}), scale) +
                   make_shapes(stype::matball01, mtype::metal02,
                               make_frame({0, 0.5f, 0}), scale) +
                   make_shapes(stype::matball01, mtype::metal03,
                               make_frame({1.25f, 0.5f, 0}), scale);
        } break;
        case stype::simple_matballs04: {
            return make_shapes(stype::matball01, mtype::glass01,
                               make_frame({-1.25f, 0.5f, 0}), scale) +
                   make_shapes(stype::matball01, mtype::glass02,
                               make_frame({0, 0.5f, 0}), scale) +
                   make_shapes(stype::matball01, mtype::glass03,
                               make_frame({1.25f, 0.5f, 0}), scale);
        } break;
        case stype::simple_random32: {
            return make_random_shapes(32, 5);
        } break;
    }
    return {};
}

std::vector<std::shared_ptr<yapp::shape>> make_random_shapes(int nshapes,
                                                             int l) {
    ym::vec3f pos[1024];
    float radius[1024];
    int levels[1024];

    assert(nshapes < 1024);

    ym::rng_pcg32 rn;
    for (auto i = 0; i < nshapes; i++) {
        auto done = false;
        while (!done) {
            auto x = -2 + 4 * next1f(&rn);
            auto z = 1 - 3 * next1f(&rn);
            radius[i] = 0.15f + ((1 - z) / 3) * ((1 - z) / 3) * 0.5f;
            pos[i] = ym::vec3f{x, radius[i], z};
            levels[i] = (int)round(log2f(powf(2, (float)l) * radius[i] / 0.5f));
            done = true;
            for (int j = 1; j < i && done; j++) {
                if (dist(pos[i], pos[j]) < radius[i] + radius[j]) done = false;
            }
        }
    }

    std::vector<stype> stypes = {stype::obj01, stype::obj02, stype::obj03,
                                 stype::matball01};
    std::vector<mtype> mtypes = {
        mtype::matte,         mtype::matte_txt,     mtype::plastic01,
        mtype::plastic02,     mtype::plastic03,     mtype::plastic01_txt,
        mtype::plastic02_txt, mtype::plastic03_txt, mtype::metal01,
        mtype::metal02,       mtype::metal03};

    std::vector<std::shared_ptr<yapp::shape>> shapes;
    for (auto i = 0; i < nshapes; i++) {
        char name[1024];
        sprintf(name, "obj%02d", i);
        auto st = stypes[(int)(next1f(&rn) * (float)stypes.size())];
        auto mt = mtypes[(int)(next1f(&rn) * (float)mtypes.size())];
        shapes += make_shapes(st, mt, make_frame(pos[i]),
                              {2 * radius[i], 2 * radius[i], 2 * radius[i]});
    }

    return shapes;
}

std::shared_ptr<yapp::environment> make_env(
    const std::string& name, std::shared_ptr<yapp::material> mat,
    const ym::frame3f& frame = make_lookat_frame({0, 0.5f, 0},
                                                 {-1.5f, 0.5f, 0})) {
    auto env = std::make_shared<yapp::environment>();
    env->name = name;
    env->mat = mat;
    env->frame = frame;
    return env;
}

enum struct etype { env00, env01 };

std::vector<std::shared_ptr<yapp::environment>> make_environments(
    etype et, mtype mt = mtype::def,
    const ym::frame3f& frame = make_lookat_frame({0, 0.5f, 0},
                                                 {-1.5f, 0.5f, 0})) {
    switch (et) {
        case etype::env00: {
            auto mat = (mt == mtype::def) ? make_material(mtype::env00)
                                          : make_material(mt);
            return {make_env("env_" + mat->name, mat, frame)};
        } break;
        case etype::env01: {
            auto mat = (mt == mtype::def) ? make_material(mtype::env01)
                                          : make_material(mt);
            return {make_env("env_" + mat->name, mat, frame)};
        } break;
    }
    return {};
}

std::shared_ptr<yapp::camera> make_camera(const std::string& name,
                                          const ym::vec3f& from,
                                          const ym::vec3f& to, float h,
                                          float a) {
    auto cam = std::make_shared<yapp::camera>();
    cam->name = name;
    cam->frame = lookat_frame3(from, to, {0, 1, 0});
    cam->aperture = a;
    cam->focus = dist(from, to);
    cam->yfov = 2 * atan(h / 2);
    cam->aspect = 16.0f / 9.0f;
    return cam;
}

std::vector<std::shared_ptr<yapp::texture>> make_random_textures() {
    const std::string txts[5] = {"grid.png", "checker.png", "rchecker.png",
                                 "colored.png", "rcolored.png"};
    std::vector<std::shared_ptr<yapp::texture>> textures;
    for (auto txt : txts) {
        textures.push_back(std::make_shared<yapp::texture>());
        textures.back()->path = txt;
    }
    return textures;
}

std::vector<std::shared_ptr<yapp::material>> make_random_materials(
    int nshapes) {
    auto textures = make_random_textures();
    std::vector<std::shared_ptr<yapp::material>> materials(nshapes);
    materials[0] = make_diffuse("floor", {1, 1, 1}, textures[0]);

    ym::rng_pcg32 rn;
    for (auto i = 1; i < nshapes; i++) {
        char name[1024];
        sprintf(name, "obj%02d", i);
        auto txt = -1;
        if (next1f(&rn) < 0.5f) {
            txt = (int)(next1f(&rn) * 6) - 1;
        }
        auto c = (txt >= 0) ? ym::vec3f{1, 1, 1}
                            : ym::vec3f{0.2f + 0.3f * next1f(&rn),
                                        0.2f + 0.3f * next1f(&rn),
                                        0.2f + 0.3f * next1f(&rn)};
        auto rs = 0.01f + 0.25f * next1f(&rn);
        auto mt = (int)(next1f(&rn) * 4);
        if (mt == 0) {
            materials[i] =
                make_diffuse(name, c, (txt < 0) ? nullptr : textures[txt]);
        } else if (mt == 1) {
            materials[i] =
                make_metal(name, c, rs, (txt < 0) ? nullptr : textures[txt]);
        } else {
            materials[i] =
                make_plastic(name, c, rs, (txt < 0) ? nullptr : textures[txt]);
        }
    }

    return materials;
}

std::vector<std::shared_ptr<yapp::shape>> make_random_rigid_shapes(
    int nshapes, int l,
    const std::vector<std::shared_ptr<yapp::material>>& materials) {
    std::vector<std::shared_ptr<yapp::shape>> shapes(nshapes);
    shapes[0] =
        make_shape("floor", materials[0], 2, yshape::stdsurface_type::uvcube,
                   make_frame({0, -0.5, 0}), {6, 0.5, 6});
    ym::vec3f pos[1024];
    float radius[1024];
    int levels[1024];

    ym::rng_pcg32 rn;
    for (int i = 1; i < nshapes; i++) {
        bool done = false;
        while (!done) {
            radius[i] = 0.1f + 0.4f * next1f(&rn);
            pos[i] = ym::vec3f{-2 + 4 * next1f(&rn), 1 + 4 * next1f(&rn),
                               -2 + 4 * next1f(&rn)};
            levels[i] = (int)round(log2f(powf(2, (float)l) * radius[i] / 0.5f));
            done = true;
            for (int j = 1; j < i && done; j++) {
                if (dist(pos[i], pos[j]) < radius[i] + radius[j]) done = false;
            }
        }
    }

    for (int i = 1; i < nshapes; i++) {
        auto name = "obj" + std::to_string(i);
        yshape::stdsurface_type stypes[2] = {
            yshape::stdsurface_type::uvspherecube,
            yshape::stdsurface_type::uvcube};
        auto stype = stypes[(int)(next1f(&rn) * 2)];
        shapes[i] =
            make_shape(name, materials[i], levels[i], stype, make_frame(pos[i]),
                       {radius[i], radius[i], radius[i]});
    }

    return shapes;
}

std::shared_ptr<yapp::scene> make_scene(
    const std::vector<std::shared_ptr<yapp::camera>>& cameras,
    const std::vector<std::shared_ptr<yapp::shape>>& shapes,
    const std::vector<std::shared_ptr<yapp::environment>>& envs = {}) {
    auto scene = std::make_shared<yapp::scene>();
    scene->cameras = cameras;
    auto materials = std::set<std::shared_ptr<yapp::material>>();
    auto textures = std::set<std::shared_ptr<yapp::texture>>();
    for (auto shp : shapes) {
        scene->shapes.push_back(shp);
        assert(shp->mat);
        materials.insert(shp->mat);
    }
    for (auto env : envs) {
        scene->environments.push_back(env);
        assert(env->mat);
        materials.insert(env->mat);
    }
    for (auto mat : materials) {
        scene->materials.push_back(mat);
        textures.insert(mat->ke_txt);
        textures.insert(mat->kd_txt);
        textures.insert(mat->ks_txt);
        textures.insert(mat->kt_txt);
        textures.insert(mat->rs_txt);
    }
    textures.erase(nullptr);
    for (auto txt : textures) {
        scene->textures.push_back(txt);
    }
    return scene;
}

using ubyte = unsigned char;
struct rgba {
    ubyte r, g, b, a;
};

std::vector<rgba> make_grid(int s) {
    std::vector<rgba> pixels(s * s);
    int g = 64;
    for (int j = 0; j < s; j++) {
        for (int i = 0; i < s; i++) {
            if (i % g == 0 || i % g == g - 1 || j % g == 0 || j % g == g - 1)
                pixels[j * s + i] = rgba{90, 90, 90, 255};
            else
                pixels[j * s + i] = rgba{128, 128, 128, 255};
        }
    }
    return pixels;
}

std::vector<rgba> make_checker(int s) {
    std::vector<rgba> pixels(s * s);
    for (int j = 0; j < s; j++) {
        for (int i = 0; i < s; i++) {
            if ((i / 64 + j / 64) % 2)
                pixels[j * s + i] = rgba{90, 90, 90, 255};
            else
                pixels[j * s + i] = rgba{128, 128, 128, 255};
        }
    }
    return pixels;
}

// http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
rgba hsv_to_rgb(ubyte h, ubyte s, ubyte v) {
    rgba rgb = {0, 0, 0, 255};
    ubyte region, remainder, p, q, t;

    if (s == 0) {
        rgb.r = v;
        rgb.g = v;
        rgb.b = v;
        return rgb;
    }

    region = h / 43;
    remainder = (h - (region * 43)) * 6;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            rgb.r = v;
            rgb.g = t;
            rgb.b = p;
            break;
        case 1:
            rgb.r = q;
            rgb.g = v;
            rgb.b = p;
            break;
        case 2:
            rgb.r = p;
            rgb.g = v;
            rgb.b = t;
            break;
        case 3:
            rgb.r = p;
            rgb.g = q;
            rgb.b = v;
            break;
        case 4:
            rgb.r = t;
            rgb.g = p;
            rgb.b = v;
            break;
        default:
            rgb.r = v;
            rgb.g = p;
            rgb.b = q;
            break;
    }

    return rgb;
}

std::vector<rgba> make_rcolored(int s) {
    std::vector<rgba> pixels(s * s);
    for (int j = 0; j < s; j++) {
        for (int i = 0; i < s; i++) {
            ubyte ph = 32 * (i / (s / 8));
            ubyte pv = 128;
            ubyte ps = 64 + 16 * (7 - j / (s / 8));
            if (i % 32 && j % 32) {
                if ((i / 64 + j / 64) % 2)
                    pv += 16;
                else
                    pv -= 16;
                if ((i / 16 + j / 16) % 2)
                    pv += 4;
                else
                    pv -= 4;
                if ((i / 4 + j / 4) % 2)
                    pv += 1;
                else
                    pv -= 1;
            } else {
                pv = 196;
                ps = 32;
            }
            pixels[j * s + i] = hsv_to_rgb(ph, ps, pv);
        }
    }
    return pixels;
}

std::vector<rgba> make_gammaramp(int s) {
    std::vector<rgba> pixels(s * s);
    for (int j = 0; j < s; j++) {
        for (int i = 0; i < s; i++) {
            auto u = j / float(s - 1);
            if (i < s / 3) u = pow(u, 2.2f);
            if (i > (s * 2) / 3) u = pow(u, 1 / 2.2f);
            auto c = (unsigned char)(u * 255);
            pixels[j * s + i] = {c, c, c, 255};
        }
    }
    return pixels;
}

std::vector<ym::vec4f> make_gammarampf(int s) {
    std::vector<ym::vec4f> pixels(s * s);
    for (int j = 0; j < s; j++) {
        for (int i = 0; i < s; i++) {
            auto u = j / float(s - 1);
            if (i < s / 3) u = pow(u, 2.2f);
            if (i > (s * 2) / 3) u = pow(u, 1 / 2.2f);
            pixels[j * s + i] = {u, u, u, 1};
        }
    }
    return pixels;
}

std::vector<rgba> make_colored(int s) {
    std::vector<rgba> pixels(s * s);
    for (int j = 0; j < s; j++) {
        for (int i = 0; i < s; i++) {
            ubyte ph = 32 * (i / (s / 8));
            ubyte pv = 128;
            ubyte ps = 64 + 16 * (7 - j / (s / 8));
            if (i % 32 && j % 32) {
                if ((i / 64 + j / 64) % 2)
                    pv += 16;
                else
                    pv -= 16;
            } else {
                pv = 196;
                ps = 32;
            }
            pixels[j * s + i] = hsv_to_rgb(ph, ps, pv);
        }
    }
    return pixels;
}

std::vector<rgba> make_rchecker(int s) {
    std::vector<rgba> pixels(s * s);
    for (int j = 0; j < s; j++) {
        for (int i = 0; i < s; i++) {
            ubyte pv = 128;
            if (i % 32 && j % 32) {
                if ((i / 64 + j / 64) % 2)
                    pv += 16;
                else
                    pv -= 16;
                if ((i / 16 + j / 16) % 2)
                    pv += 4;
                else
                    pv -= 4;
                if ((i / 4 + j / 4) % 2)
                    pv += 1;
                else
                    pv -= 1;
            } else {
                pv = 196;
            }
            pixels[j * s + i] = rgba{pv, pv, pv, 255};
        }
    }
    return pixels;
}

#define sqr(x) ((x) * (x))

std::vector<ym::vec4f> make_sunsky_hdr(int w, int h, float sun_theta,
                                       float turbidity, ym::vec3f ground,
                                       float scale, bool include_ground) {
    std::vector<ym::vec4f> rgba(w * h);
    ArHosekSkyModelState* skymodel_state[3] = {
        arhosek_rgb_skymodelstate_alloc_init(turbidity, ground[0], sun_theta),
        arhosek_rgb_skymodelstate_alloc_init(turbidity, ground[0], sun_theta),
        arhosek_rgb_skymodelstate_alloc_init(turbidity, ground[0], sun_theta),
    };
    auto sun_phi = ym::pif;
    auto sun_w = ym::vec3f{cosf(sun_phi) * sinf(sun_theta),
                           sinf(sun_phi) * sinf(sun_theta), cosf(sun_theta)};
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            auto theta = ym::pif * (j + 0.5f) / h;
            auto phi = 2 * ym::pif * (i + 0.5f) / w;
            if (include_ground)
                theta = ym::clamp(theta, 0.0f, ym::pif / 2 - 0.001f);
            auto pw =
                ym::vec3f{std::cos(phi) * std::sin(theta),
                          std::sin(phi) * std::sin(theta), std::cos(theta)};
            auto gamma =
                std::acos(ym::clamp(dot(sun_w, pw), (float)-1, (float)1));
            auto sky = ym::vec3f{(float)(arhosek_tristim_skymodel_radiance(
                                     skymodel_state[0], theta, gamma, 0)),
                                 (float)(arhosek_tristim_skymodel_radiance(
                                     skymodel_state[1], theta, gamma, 1)),
                                 (float)(arhosek_tristim_skymodel_radiance(
                                     skymodel_state[2], theta, gamma, 2))};
            rgba[j * w + i] = {scale * sky[0], scale * sky[1], scale * sky[2],
                               1};
        }
    }
    arhosekskymodelstate_free(skymodel_state[0]);
    arhosekskymodelstate_free(skymodel_state[1]);
    arhosekskymodelstate_free(skymodel_state[2]);
    return rgba;
}

void save_image(const std::string& filename, const std::string& dirname,
                const rgba* pixels, int s) {
    std::string path = std::string(dirname) + "/" + std::string(filename);
    stbi_write_png(path.c_str(), s, s, 4, pixels, s * 4);
}

void save_image_hdr(const std::string& filename, const std::string& dirname,
                    const ym::vec4f* pixels, int w, int h) {
    std::string path = std::string(dirname) + "/" + std::string(filename);
    stbi_write_hdr(path.c_str(), w, h, 4, (float*)pixels);
}

void save_scene(const std::string& filename, const std::string& dirname,
                const std::shared_ptr<yapp::scene>& scn) {
    yapp::save_scene(dirname + "/" + filename, scn);
    yapp::save_scene(dirname + "/" + ycmd::get_basename(filename) + ".gltf",
                     scn);
}

std::vector<std::shared_ptr<yapp::camera>> make_simple_cameras() {
    return {make_camera("cam", {0, 1.5f, 5}, {0, 0.5f, 0}, 0.5f, 0),
            make_camera("cam_dof", {0, 1.5f, 5}, {0, 0.5, 0}, 0.5f, 0.1f)};
}

// http://graphics.cs.williams.edu/data
// http://www.graphics.cornell.edu/online/box/data.html
std::shared_ptr<yapp::scene> make_cornell_box_scene() {
    std::vector<std::shared_ptr<yapp::camera>> cameras = {
        make_camera("cb_cam", {0, 1, 4}, {0, 1, 0}, 0.7f, 0)};
    std::vector<std::shared_ptr<yapp::material>> materials = {
        make_diffuse("cb_white", {0.725f, 0.71f, 0.68f}),
        make_diffuse("cb_red", {0.63f, 0.065f, 0.05f}),
        make_diffuse("cb_green", {0.14f, 0.45f, 0.091f}),
        make_emission("cb_light", {17, 12, 4}),
    };
    std::vector<std::shared_ptr<yapp::shape>> shapes = {
        make_shape("cb_floor", materials[0], 0, yshape::stdsurface_type::uvquad,
                   make_frame(ym::zero3f, {-90, 0, 0})),
        make_shape("cb_ceiling", materials[0], 0,
                   yshape::stdsurface_type::uvquad,
                   make_frame({0, 2, 0}, {90, 0, 0})),
        make_shape("cb_back", materials[0], 0, yshape::stdsurface_type::uvquad,
                   make_frame({0, 1, -1}, ym::zero3f)),
        make_shape("cb_back", materials[2], 0, yshape::stdsurface_type::uvquad,
                   make_frame({+1, 1, 0}, {0, -90, 0})),
        make_shape("cb_back", materials[1], 0, yshape::stdsurface_type::uvquad,
                   make_frame({-1, 1, 0}, {0, 90, 0})),
        make_shape(
            "cb_tallbox", materials[0], 0, yshape::stdsurface_type::uvcube,
            make_frame({-0.33f, 0.6f, -0.29f}, {0, 15, 0}), {0.3f, 0.6f, 0.3f}),
        make_shape(
            "cb_shortbox", materials[0], 0, yshape::stdsurface_type::uvcube,
            make_frame({0.33f, 0.3f, 0.33f}, {0, -15, 0}), {0.3f, 0.3f, 0.3f}),
        make_shape("cb_light", materials[3], 0, yshape::stdsurface_type::uvquad,
                   make_frame({0, 1.999f, 0}, {90, 0, 0}),
                   {0.25f, 0.25f, 0.25f})};
    return make_scene(cameras, shapes);
}

std::shared_ptr<yapp::scene> make_envmap_scene(bool as_shape, bool use_map) {
    if (!as_shape) {
        if (use_map) {
            return make_scene(make_simple_cameras(),
                              make_shapes(stype::floor02) +
                                  make_shapes(stype::simple_objs_notxt),
                              make_environments(etype::env01));
        } else {
            return make_scene(make_simple_cameras(),
                              make_shapes(stype::floor02) +
                                  make_shapes(stype::simple_objs_notxt),
                              make_environments(etype::env00));
        }
    } else {
        if (use_map) {
            return make_scene(make_simple_cameras(),
                              make_shapes(stype::floor02) +
                                  make_shapes(stype::simple_objs_notxt) +
                                  make_shapes(stype::envsphere01));
        } else {
            return make_scene(make_simple_cameras(),
                              make_shapes(stype::floor02) +
                                  make_shapes(stype::simple_objs_notxt) +
                                  make_shapes(stype::envsphere00));
        }
    }
}

std::shared_ptr<yapp::scene> make_rigid_scene(int config) {
    std::vector<std::shared_ptr<yapp::camera>> cameras = {
        make_camera("cam", {5, 5, 5}, {0, 0.5f, 0}, 0.5f, 0),
        make_camera("cam_dof", {5, 5, 5}, {0, 0.5f, 0}, 0.5f, 0.1f)};
    std::vector<std::shared_ptr<yapp::shape>> shapes;

    std::vector<std::shared_ptr<yapp::texture>> textures = {
        make_texture("grid.png"), make_texture("checker.png")};

    if (config == 0 || config == 1) {
        std::vector<std::shared_ptr<yapp::material>> materials = {
            make_diffuse("floor", {1, 1, 1}, 0),
            make_plastic("obj", {1, 1, 1}, 0.1f, textures[1])};
        shapes = {
            (config)
                ? make_shape("floor", materials[0], 2,
                             yshape::stdsurface_type::uvcube,
                             make_frame({0, -2.5, 0}, {30, 0, 0}), {6, 0.5f, 6})
                : make_shape(
                      "floor", materials[0], 4, yshape::stdsurface_type::uvcube,
                      make_frame({0, -0.5f, 0}, {0, 0, 0}), {6, 0.5f, 6}),
            make_shape(
                "obj01", materials[1], 2, yshape::stdsurface_type::uvcube,
                make_frame({-1.25f, 0.5f, 0}, {0, 0, 0}), {0.5f, 0.5f, 0.5f}),
            make_shape("obj02", materials[1], 3,
                       yshape::stdsurface_type::uvspherecube,
                       make_frame({0, 1, 0}, {0, 0, 0}), {0.5f, 0.5f, 0.5f}),
            make_shape(
                "obj03", materials[1], 2, yshape::stdsurface_type::uvcube,
                make_frame({1.25f, 1.5f, 0}, {0, 0, 0}), {0.5f, 0.5f, 0.5f}),
            make_shape("obj11", materials[1], 2,
                       yshape::stdsurface_type::uvcube,
                       make_frame({-1.25f, 0.5f, 1.5f}, {0, 45, 0}),
                       {0.5f, 0.5f, 0.5f}),
            make_shape(
                "obj12", materials[1], 3, yshape::stdsurface_type::uvspherecube,
                make_frame({0, 1, 1.5f}, {45, 0, 0}), {0.5f, 0.5f, 0.5f}),
            make_shape("obj13", materials[1], 2,
                       yshape::stdsurface_type::uvcube,
                       make_frame({1.25f, 1.5f, 1.5f}, {45, 0, 45}),
                       {0.5f, 0.5f, 0.5f}),
            make_shape("obj21", materials[1], 2,
                       yshape::stdsurface_type::uvcube,
                       make_frame({-1.25f, 0.5f, -1.5f}, {0, 0, 0}),
                       {0.5f, 0.5f, 0.5f}),
            make_shape(
                "obj22", materials[1], 3, yshape::stdsurface_type::uvspherecube,
                make_frame({0, 1, -1.5f}, {22.5, 0, 0}), {0.5f, 0.5f, 0.5f}),
            make_shape("obj23", materials[1], 2,
                       yshape::stdsurface_type::uvcube,
                       make_frame({1.25f, 1.5f, -1.5f}, {22.5f, 0, 22.5f}),
                       {0.5f, 0.5f, 0.5f})};
    } else if (config == 2) {
        shapes = make_random_rigid_shapes(128, 1, make_random_materials(128));
    } else {
        assert(false);
    }

    shapes.push_back(make_point("light01",
                                make_emission("light01", {100, 100, 100}),
                                make_frame({0.7f, 4, 3})));
    shapes.push_back(make_point("light02",
                                make_emission("light02", {100, 100, 100}),
                                make_frame({-0.7f, 4, 3})));

    return make_scene(cameras, shapes);
}

int main(int argc, char* argv[]) {
    // command line params
    auto parser = ycmd::make_parser(argc, argv, "make tests");
    auto dirname =
        ycmd::parse_args(parser, "dirname", "directory name", ".", true);
    ycmd::check_parser(parser);

// make directories
#ifndef _MSC_VER
    auto cmd = "mkdir -p " + dirname;
#else
    auto cmd = "mkdir " + dirname;
#endif
    system(cmd.c_str());

    // simple scene ------------------------------
    auto stypes = std::vector<std::pair<std::string, stype>>{
        {"basic", stype::simple_objs_notxt},
        {"simple", stype::simple_objs},
        {"mat01", stype::simple_matballs01},
        {"mat02", stype::simple_matballs02},
        {"mat03", stype::simple_matballs03},
        {"mat04", stype::simple_matballs04},
        {"rand32", stype::simple_random32},
        {"lines", stype::simple_lines},
        {"points", stype::simple_points},
    };
    auto ftype = stype::floor02;
    for (auto stype : stypes) {
        printf("generating %s scenes ...\n", stype.first.c_str());
        save_scene(stype.first + "_pointlight.obj", dirname,
                   make_scene(make_simple_cameras(),
                              make_shapes(ftype) + make_shapes(stype.second) +
                                  make_shapes(stype::simple_pointlights)));
        save_scene(stype.first + "_arealight.obj", dirname,
                   make_scene(make_simple_cameras(),
                              make_shapes(ftype) + make_shapes(stype.second) +
                                  make_shapes(stype::simple_arealights)));
        save_scene(stype.first + "_envlight.obj", dirname,
                   make_scene(make_simple_cameras(),
                              make_shapes(ftype) + make_shapes(stype.second),
                              make_environments(etype::env01)));
    }

    // env scene ------------------------------
    printf("generating envmaps scenes ...\n");
    save_scene("env_shape_const.obj", dirname, make_envmap_scene(true, false));
    save_scene("env_shape_map.obj", dirname, make_envmap_scene(true, true));
    save_scene("env_inf_const.obj", dirname, make_envmap_scene(false, false));
    save_scene("env_inf_map.obj", dirname, make_envmap_scene(false, true));

    // cornell box ------------------------------
    printf("generating cornell box scenes ...\n");
    save_scene("cornell_box.obj", dirname, make_cornell_box_scene());

    // rigid body scenes ------------------------
    printf("generating rigid body scenes ...\n");
    save_scene("rigid_01.obj", dirname, make_rigid_scene(0));
    save_scene("rigid_02.obj", dirname, make_rigid_scene(1));
    // save_scene("rigid_03.obj", dirname, make_rigid_scene(2));

    // textures ---------------------------------
    printf("generating simple textures ...\n");
    save_image("grid.png", dirname, make_grid(512).data(), 512);
    save_image("checker.png", dirname, make_checker(512).data(), 512);
    save_image("rchecker.png", dirname, make_rchecker(512).data(), 512);
    save_image("colored.png", dirname, make_colored(512).data(), 512);
    save_image("rcolored.png", dirname, make_rcolored(512).data(), 512);
    save_image("gamma.png", dirname, make_gammaramp(512).data(), 512);
    save_image_hdr("gamma.hdr", dirname, make_gammarampf(512).data(), 512, 512);
    printf("generating envmaps textures ...\n");
    save_image_hdr("env.hdr", dirname,
                   make_sunsky_hdr(1024, 512, 0.8f, 8,
                                   ym::vec3f{0.2f, 0.2f, 0.2f}, 1 / powf(2, 6),
                                   true)
                       .data(),
                   1024, 512);
    save_image_hdr("env01.hdr", dirname,
                   make_sunsky_hdr(1024, 512, 0.8f, 8,
                                   ym::vec3f{0.2f, 0.2f, 0.2f}, 1 / powf(2, 6),
                                   true)
                       .data(),
                   1024, 512);
}
