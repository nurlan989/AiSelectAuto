#include "preprocessor.h"

Preprocessor::Result Preprocessor::process(
    const cv::Mat& image,
    int inputWidth,
    int inputHeight)
{
    Result result;

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(inputWidth, inputHeight));

    result.scaleX = static_cast<float>(image.cols) / inputWidth;
    result.scaleY = static_cast<float>(image.rows) / inputHeight;

    resized.convertTo(resized, CV_32F, 1.0 / 255.0);

    result.tensor.resize(inputWidth * inputHeight * 3);

    int idx = 0;

    for (int c = 0; c < 3; c++) {
        for (int y = 0; y < inputHeight; y++) {
            for (int x = 0; x < inputWidth; x++) {
                result.tensor[idx++] =
                    resized.at<cv::Vec3f>(y, x)[c];
            }
        }
    }

    result.shape = {1, 3, inputHeight, inputWidth};

    return result;
}

Frame Preprocessor::run(const Frame& frame, const cv::Size& size)
{

    Frame out = frame;

    cv::Mat resized;

    cv::resize(frame.mat, resized, size);

    out.mat = resized;

    return out;

}
