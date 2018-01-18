//
// Created by somefive on 18-1-18.
//

#include "SaliencyDetect.h"
#include <iostream>
#include <fstream>

#define K 64
#define C 3

using std::ofstream;
using std::cout;
using std::endl;
using std::pair;

Mat source;
int rows, cols;

double color_distance(const Mat & src, int row1, int col1, int row2, int col2) {
    auto & c1 = src.at<Vec3b>(row1, col1), & c2 = src.at<Vec3b>(row2, col2);
    double dc0 = (c1[0] - c2[0] + 0.0) / 255, dc1 = (c1[1] - c2[1] + 0.0) / 255, dc2 = (c1[2] - c2[2] + 0.0) / 255;
    return sqrt(dc0 * dc0 + dc1 * dc1 + dc2 * dc2);
}

double distance(const Mat & src, int row1, int col1, int row2, int col2) {
    double color_dis = color_distance(src, row1, col1, row2, col2);
    double dRow = (row1 - row2 + 0.0) / rows, dCol = (col1 - col2 + 0.0) / cols;
    double xy_dis = sqrt(dRow * dRow + dCol * dCol);
    return color_dis / (1 + C * xy_dis);
}
double salient(const Mat & src, int r, int c) {
    std::vector<double> diffs;
    for (int row = 0; row < rows; ++row)
        for (int col = 0; col < cols; ++col)
            diffs.push_back(distance(src, r, c, row, col));
    std::sort(diffs.begin(), diffs.end());
    double sum = 0;
    int n = 0;
    for (n = 0; n <= K && n < diffs.size(); ++n)
        sum += diffs[n];
    return 1 - exp(-sum/n);
}
Mat saliencyMatrix(const Mat & _src, int u) {
    Mat src = _src.clone();
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int n = 0;
            int l = 0, a = 0, b = 0;
            for (int r = row - u; r <= row + u; ++r) {
                if (r < 0 || r >= rows) continue;
                for (int c = col - u; c <= col + u; ++c) {
                    if (c < 0 || c >= cols) continue;
                    ++n;
                    l += src.at<Vec3b>(r, c)[0];
                    a += src.at<Vec3b>(r, c)[1];
                    b += src.at<Vec3b>(r, c)[2];
                }
            }
            src.at<Vec3b>(row, col) = Vec3b(static_cast<uchar>(l / n), static_cast<uchar>(a / n), static_cast<uchar>(b / n));
        }
    }
    Mat tg = Mat(rows, cols, CV_8U);
    Mat mid = Mat(rows, cols, CV_64F);
    double _max = 0;
    printf("generating at u=%d\n", u);
    for (int row = 0; row < rows; ++row) {
        printf("\r%d/%d", row, rows);
        std::flush(std::cout);
        for (int col = 0; col < cols; ++col) {
            double value = salient(src, row, col);
            mid.at<double>(row, col) = value;
            if (value > _max) _max = value;
        }
    }
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            double value = mid.at<double>(row, col) / _max;
            tg.at<uchar>(row, col) = static_cast<uchar>(value * 255);
        }
    }
    printf("\n");
    return tg;
}

void exec(const String &filename, const String& outFile, int u) {
    source = imread(filename);
    cvtColor(source, source, cv::ColorConversionCodes::COLOR_RGB2Lab);
    rows = source.rows;
    cols = source.cols;
    printf("rows: %d, cols: %d\n", rows, cols);

    Mat tg4 = saliencyMatrix(source, u);
    imwrite(filename + ".4.png", tg4);
    Mat tg2 = saliencyMatrix(source, u/2);
    imwrite(filename + ".2.png", tg2);
    Mat tg0 = saliencyMatrix(source, 0);
    imwrite(filename + ".0.png", tg0);
    Mat tg = Mat(rows, cols, CV_8U);

    std::vector<pair<pair<int, int>, uchar>> mainPart;
    uchar _max = 0;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            auto avg = (uchar) (
                ((int) tg4.at<uchar>(row, col) + (int) tg2.at<uchar>(row, col) + (int) tg0.at<uchar>(row, col)) / 3);
            tg.at<uchar>(row, col) = avg;
            if (avg > _max) _max = avg;
        }
    }
    printf("tg max: %d", _max);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            tg.at<uchar>(row, col) = static_cast<uchar>(tg.at<uchar>(row, col) * 255.0 / _max);
            if (tg.at<uchar>(row, col) > 204) mainPart.emplace_back(pair<int, int>(row, col), tg.at<uchar>(row, col));
        }
    }
    imwrite(filename + ".tg.png", tg);

    Mat S = tg.clone();
    printf("len:%d\n", static_cast<int>(mainPart.size()));
    printf("optimizing\n");
    if (!mainPart.empty()) {
        for (int row = 0; row < rows; ++row) {
            printf("\r%d/%d", row, rows);
            std::flush(std::cout);
            for (int col = 0; col < cols; ++col) {
                uchar value = S.at<uchar>(row, col);
                if (value > 204) continue;
                double dis = 1;
                for (auto p: mainPart) {
                    double dRow = (p.first.first - row + 0.0) / rows;
                    double dCol = (p.first.second - col + 0.0) / cols;
                    double _dis = sqrt(dRow * dRow + dCol * dCol);
                    if (_dis < dis) dis = _dis;
                }
                S.at<uchar>(row, col) = static_cast<uchar>(value * (1.0 - dis));
            }
        }
    }
    printf("\n");
    imwrite(filename + ".fin.png", S);
    imwrite(outFile, S);
}