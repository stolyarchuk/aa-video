#pragma once

#include <array>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <opencv2/opencv.hpp>

/**
 * @brief Shared utility functions and constants for the AA Video Processing
 * System
 *
 * Contains common functions, data structures, and constants used across
 * client and server components. Includes drawing utilities, color constants,
 * and shared type definitions.
 */
namespace aa::shared {

/**
 * @brief Draw a semi-transparent rectangle on an image
 * @param frame The image to draw on
 * @param left Left coordinate of the rectangle
 * @param top Top coordinate of the rectangle
 * @param right Right coordinate of the rectangle
 * @param bottom Bottom coordinate of the rectangle
 * @param color Color of the rectangle
 * @param alpha Transparency value (0.0 = transparent, 1.0 = opaque)
 */
void DrawSemiTransparentRect(cv::Mat& frame, int left, int top, int right,
                             int bottom, const cv::Scalar& color, float alpha);

void DrawColoredRect(cv::Mat& frame, int left, int top, int right, int bottom,
                     const cv::Scalar& color);

/**
 * @brief Draw a prediction box with label on an image
 * @param class_id The class ID of the detected object
 * @param conf Confidence score of the detection
 * @param left Left coordinate of the bounding box
 * @param top Top coordinate of the bounding box
 * @param right Right coordinate of the bounding box
 * @param bottom Bottom coordinate of the bounding box
 * @param frame The image to draw on
 */
void DrawBoundingBox(cv::Mat& frame, int left, int top, int right, int bottom,
                     int class_id, float conf, const cv::Scalar& color,
                     bool filled = false);

}  // namespace aa::shared
