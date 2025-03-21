#pragma once

#include "common.h"
#include "fwd.h"

struct ImageMetrics
{
    float l1;
    float l1_avg;
    float l2;
    float mse;

    float psnr;
};

constexpr int kImageCompareErrorSize    = 1;
constexpr int kImageCompareErrorChannel = 2;

int is_metric_computable(const Image &reference, const Image &img);

ImageMetrics compute_metrics(const Image &reference, const Image &img);

void draw_metric(ConstImagePtr pRef, ConstImagePtr pImg);
