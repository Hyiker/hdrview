#include "image_compare.h"
#include "app.h"

#include "imgui_ext.h"
#include "imgui_internal.h"

#include <cmath>

#include "image.h"

int is_metric_computable(const Image &reference, const Image &img)
{
    if (!(reference.size().x == img.size().x && reference.size().y == img.size().y))
        return kImageCompareErrorSize;

    if (reference.channels.size() != img.channels.size())
        return kImageCompareErrorChannel;

    return 0;
}

ImageMetrics compute_metrics(const Image &reference, const Image &img)
{
    ImageMetrics metrics{};

    float sum_l1          = 0.0f;
    float sum_l2_squared  = 0.0f;
    float max_pixel_value = 0.0f;

    if (img.size().x != reference.size().x || img.size().y != reference.size().y)
    {
        metrics.l1     = -1.0f;
        metrics.l1_avg = -1.0f;
        metrics.l2     = -1.0f;
        metrics.mse    = -1.0f;
        metrics.psnr   = -1.0f;
        return metrics;
    }

    int total_pixels = img.size().x * img.size().y;

    for (int y = 0; y < img.size().y; ++y)
        for (int x = 0; x < img.size().x; ++x)
        {
            float4 imgV = img.rgba_pixel(int2(x, y), Target_Primary);
            float4 refV = reference.rgba_pixel(int2(x, y), Target_Primary);

            float4 diff = imgV - refV;

            sum_l1 += fabsf(diff.x) + fabsf(diff.y) + fabsf(diff.z);
            sum_l2_squared += diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

            max_pixel_value = fmaxf(max_pixel_value, fmaxf(fmaxf(refV.x, refV.y), refV.z));
        }

    metrics.l1     = sum_l1;
    metrics.l1_avg = sum_l1 / (total_pixels * 3.0f);
    metrics.mse    = sum_l2_squared / (total_pixels * 3.0f);
    metrics.l2     = sqrtf(metrics.mse);

    if (metrics.mse > 0.0f && max_pixel_value > 0.0f)
    {
        metrics.psnr = 20.0f * log10f(max_pixel_value / sqrtf(metrics.mse));
    }
    else
    {
        metrics.psnr = INFINITY;
    }

    return metrics;
}

struct MetricCache
{
    void        *pRef;
    void        *pImage;
    ImageMetrics metrics;
};

MetricCache cache;

void draw_metric(ConstImagePtr pRef, ConstImagePtr pImg)
{
    auto sans_font = hdrview()->font("sans regular", 14);
    auto bold_font = hdrview()->font("sans bold", 14);
    auto mono_font = hdrview()->font("mono regular", 14);

    auto property_name = [bold_font](const string &text)
    {
        ImGui::PushFont(bold_font);
        ImGui::TextUnformatted(text);
        ImGui::PopFont();
    };
    auto property_value = [](const string &text, ImFont *font, bool wrapped = false)
    {
        ImGui::SameLine();
        ImGui::PushFont(font);

        float avail_w = ImGui::GetContentRegionAvail().x;
        float text_w  = ImGui::CalcTextSize(text.c_str()).x;

        if (text_w < avail_w)
        {
            // place it on the same line as the property name
            ImGui::TextUnformatted(text);
        }
        else
        {
            // place it indented on the next line
            ImGui::NewLine();
            ImGui::Indent();
            if (wrapped)
                ImGui::TextWrapped("%s", text.c_str());
            else
                ImGui::TextUnformatted(text);
            ImGui::Unindent();
        }
        ImGui::PopFont();
    };

    int error_code = is_metric_computable(*pRef, *pImg);
    if (error_code != 0)
    {
        if (error_code == kImageCompareErrorSize)
        {
            property_name("Error: Image sizes don't match");
        }
        else if (error_code == kImageCompareErrorChannel)
        {
            property_name("Error: Channel counts don't match");
        }
        else
        {
            property_name("Unknown error");
        }
        return;
    }

    // Check cache
    if (cache.pRef != pRef.get() || cache.pImage != pImg.get())
    {
        // Update cache
        cache.pRef    = (void *)pRef.get();
        cache.pImage  = (void *)pImg.get();
        cache.metrics = compute_metrics(*pRef, *pImg);
    }

    const ImageMetrics &metrics = cache.metrics;

    // Display metrics
    property_name("L1:");
    property_value(fmt::format("{:.6f}", metrics.l1), mono_font);

    property_name("L1 (avg):");
    property_value(fmt::format("{:.6f}", metrics.l1_avg), mono_font);

    property_name("L2:");
    property_value(fmt::format("{:.6f}", metrics.l2), mono_font);

    property_name("MSE:");
    property_value(fmt::format("{:.6f}", metrics.mse), mono_font);

    if (std::isinf(metrics.psnr))
    {
        property_name("PSNR:");
        property_value("Infty", mono_font);
    }
    else
    {
        property_name("PSNR:");
        property_value(fmt::format("{:.2f} dB", metrics.psnr), mono_font);
    }
}
