#include "frame.h"

Frame Frame::fromQImage(const QImage& image)
{
    QImage img = image.convertToFormat(QImage::Format_RGB888);

    cv::Mat mat(
        img.height(),
        img.width(),
        CV_8UC3,
        const_cast<uchar*>(img.bits()),
        img.bytesPerLine()
    );

    cv::Mat copy = mat.clone();

    return Frame(copy);
}