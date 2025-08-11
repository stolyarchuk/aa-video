#pragma once

#include <opencv2/core/types.hpp>

namespace aa::shared {

struct Detection {
  cv::Rect bbox;
  int class_id;
  float confidence;
};

/**
 * @brief COCO dataset class names (80 classes)
 */
inline constexpr std::array<std::string_view, 80> kCocoClasses = {
    "person",        "bicycle",      "car",
    "motorcycle",    "airplane",     "bus",
    "train",         "truck",        "boat",
    "traffic light", "fire hydrant", "stop sign",
    "parking meter", "bench",        "bird",
    "cat",           "dog",          "horse",
    "sheep",         "cow",          "elephant",
    "bear",          "zebra",        "giraffe",
    "backpack",      "umbrella",     "handbag",
    "tie",           "suitcase",     "frisbee",
    "skis",          "snowboard",    "sports ball",
    "kite",          "baseball bat", "baseball glove",
    "skateboard",    "surfboard",    "tennis racket",
    "bottle",        "wine glass",   "cup",
    "fork",          "knife",        "spoon",
    "bowl",          "banana",       "apple",
    "sandwich",      "orange",       "broccoli",
    "carrot",        "hot dog",      "pizza",
    "donut",         "cake",         "chair",
    "couch",         "potted plant", "bed",
    "dining table",  "toilet",       "tv",
    "laptop",        "mouse",        "remote",
    "keyboard",      "cell phone",   "microwave",
    "oven",          "toaster",      "sink",
    "refrigerator",  "book",         "clock",
    "vase",          "scissors",     "teddy bear",
    "hair drier",    "toothbrush"};

struct Color {
  inline static const cv::Scalar kAliceBlue{255, 248, 240};
  inline static const cv::Scalar kAntiqueWhite{215, 235, 250};
  inline static const cv::Scalar kAqua{255, 255, 0};
  inline static const cv::Scalar kAquamarine{212, 255, 127};
  inline static const cv::Scalar kAzure{255, 255, 240};
  inline static const cv::Scalar kBeige{220, 245, 245};
  inline static const cv::Scalar kBisque{196, 228, 255};
  inline static const cv::Scalar kBlack{0, 0, 0};
  inline static const cv::Scalar kBlanchedAlmond{205, 235, 255};
  inline static const cv::Scalar kBlue{255, 0, 0};
  inline static const cv::Scalar kBlueViolet{226, 43, 138};
  inline static const cv::Scalar kBrown{42, 42, 165};
  inline static const cv::Scalar kBurlyWood{135, 184, 222};
  inline static const cv::Scalar kCadetBlue{160, 158, 95};
  inline static const cv::Scalar kChartreuse{0, 255, 127};
  inline static const cv::Scalar kChocolate{30, 105, 210};
  inline static const cv::Scalar kCoral{80, 127, 255};
  inline static const cv::Scalar kCornflowerBlue{237, 149, 100};
  inline static const cv::Scalar kCornsilk{220, 248, 255};
  inline static const cv::Scalar kCrimson{60, 20, 220};
  inline static const cv::Scalar kCyan{255, 255, 0};
  inline static const cv::Scalar kDarkBlue{139, 0, 0};
  inline static const cv::Scalar kDarkCyan{139, 139, 0};
  inline static const cv::Scalar kDarkGoldenrod{11, 134, 184};
  inline static const cv::Scalar kDarkGray{169, 169, 169};
  inline static const cv::Scalar kDarkGreen{0, 100, 0};
  inline static const cv::Scalar kDarkGrey{169, 169, 169};
  inline static const cv::Scalar kDarkKhaki{107, 183, 189};
  inline static const cv::Scalar kDarkMagenta{139, 0, 139};
  inline static const cv::Scalar kDarkOliveGreen{47, 107, 85};
  inline static const cv::Scalar kDarkOrange{0, 140, 255};
  inline static const cv::Scalar kDarkOrchid{204, 50, 153};
  inline static const cv::Scalar kDarkRed{0, 0, 139};
  inline static const cv::Scalar kDarkSalmon{122, 150, 233};
  inline static const cv::Scalar kDarkSeaGreen{143, 188, 143};
  inline static const cv::Scalar kDarkSlateBlue{139, 61, 72};
  inline static const cv::Scalar kDarkSlateGray{79, 79, 47};
  inline static const cv::Scalar kDarkSlateGrey{79, 79, 47};
  inline static const cv::Scalar kDarkTurquoise{209, 206, 0};
  inline static const cv::Scalar kDarkViolet{211, 0, 148};
  inline static const cv::Scalar kDeepPink{147, 20, 255};
  inline static const cv::Scalar kDeepSkyBlue{255, 191, 0};
  inline static const cv::Scalar kDimGray{105, 105, 105};
  inline static const cv::Scalar kDimGrey{105, 105, 105};
  inline static const cv::Scalar kDodgerBlue{255, 144, 30};
  inline static const cv::Scalar kFireBrick{34, 34, 178};
  inline static const cv::Scalar kFloralwhite{240, 250, 255};
  inline static const cv::Scalar kForestGreen{34, 139, 34};
  inline static const cv::Scalar kFuchsia{255, 0, 255};
  inline static const cv::Scalar kGainsboro{220, 220, 220};
  inline static const cv::Scalar kGhostWhite{255, 248, 248};
  inline static const cv::Scalar kGold{0, 215, 255};
  inline static const cv::Scalar kGoldenrod{32, 165, 218};
  inline static const cv::Scalar kGray{128, 128, 128};
  inline static const cv::Scalar kGreen{0, 128, 0};
  inline static const cv::Scalar kGreenYellow{47, 255, 173};
  inline static const cv::Scalar kGrey{128, 128, 128};
  inline static const cv::Scalar kHoneydew{240, 255, 240};
  inline static const cv::Scalar kHotPink{180, 105, 255};
  inline static const cv::Scalar kIndianRed{92, 92, 205};
  inline static const cv::Scalar kIndigo{130, 0, 75};
  inline static const cv::Scalar kIvory{240, 255, 255};
  inline static const cv::Scalar kKhaki{140, 230, 240};
  inline static const cv::Scalar kLavender{250, 230, 230};
  inline static const cv::Scalar kLavenderBlush{245, 240, 255};
  inline static const cv::Scalar kLawnGreen{0, 252, 124};
  inline static const cv::Scalar kLemonChiffon{205, 250, 255};
  inline static const cv::Scalar kLightBlue{230, 216, 173};
  inline static const cv::Scalar kLightCoral{128, 128, 240};
  inline static const cv::Scalar kLightCyan{255, 255, 224};
  inline static const cv::Scalar kLightGoldenrodYellow{210, 250, 250};
  inline static const cv::Scalar kLightGray{211, 211, 211};
  inline static const cv::Scalar kLightGreen{144, 238, 144};
  inline static const cv::Scalar kLightGrey{211, 211, 211};
  inline static const cv::Scalar kLightPink{193, 182, 255};
  inline static const cv::Scalar kLightSalmon{122, 160, 255};
  inline static const cv::Scalar kLightSeaGreen{170, 178, 32};
  inline static const cv::Scalar kLightSkyBlue{250, 206, 135};
  inline static const cv::Scalar kLightSlateGray{153, 136, 119};
  inline static const cv::Scalar kLightSlateGrey{153, 136, 119};
  inline static const cv::Scalar kLightSteelBlue{222, 196, 176};
  inline static const cv::Scalar kLightYellow{224, 255, 255};
  inline static const cv::Scalar kLime{0, 255, 0};
  inline static const cv::Scalar kLimeGreen{50, 205, 50};
  inline static const cv::Scalar kLinen{230, 240, 250};
  inline static const cv::Scalar kMagenta{255, 0, 255};
  inline static const cv::Scalar kMaroon{0, 0, 128};
  inline static const cv::Scalar kMediumAquamarine{170, 205, 102};
  inline static const cv::Scalar kMediumBlue{205, 0, 0};
  inline static const cv::Scalar kMediumOrchid{211, 85, 186};
  inline static const cv::Scalar kMediumPurple{219, 112, 147};
  inline static const cv::Scalar kMediumSeaGreen{113, 179, 60};
  inline static const cv::Scalar kMediumSlateBlue{238, 104, 123};
  inline static const cv::Scalar kMediumSpringGreen{154, 250, 0};
  inline static const cv::Scalar kMediumTurquoise{204, 209, 72};
  inline static const cv::Scalar kMediumVioletRed{133, 21, 199};
  inline static const cv::Scalar kMidnightBlue{112, 25, 25};
  inline static const cv::Scalar kMintCream{250, 255, 245};
  inline static const cv::Scalar kMistyRose{225, 228, 255};
  inline static const cv::Scalar kMoccasin{181, 228, 255};
  inline static const cv::Scalar kNavajoWhite{173, 222, 255};
  inline static const cv::Scalar kNavy{128, 0, 0};
  inline static const cv::Scalar kOldLace{230, 245, 253};
  inline static const cv::Scalar kOlive{0, 128, 128};
  inline static const cv::Scalar kOliveDrab{35, 142, 107};
  inline static const cv::Scalar kOrange{0, 165, 255};
  inline static const cv::Scalar kOrangeRed{0, 69, 255};
  inline static const cv::Scalar kOrchid{214, 112, 218};
  inline static const cv::Scalar kPaleGoldenrod{170, 232, 238};
  inline static const cv::Scalar kPaleGreen{152, 251, 152};
  inline static const cv::Scalar kPaleTurquoise{238, 238, 175};
  inline static const cv::Scalar kPaleVioletRed{147, 112, 219};
  inline static const cv::Scalar kPapayaWhip{213, 239, 255};
  inline static const cv::Scalar kPeachPuff{185, 218, 255};
  inline static const cv::Scalar kPeru{63, 133, 205};
  inline static const cv::Scalar kPink{203, 192, 255};
  inline static const cv::Scalar kPlum{221, 160, 221};
  inline static const cv::Scalar kPowderBlue{230, 224, 176};
  inline static const cv::Scalar kPurple{128, 0, 128};
  inline static const cv::Scalar kRed{0, 0, 255};
  inline static const cv::Scalar kRosyBrown{143, 143, 188};
  inline static const cv::Scalar kRoyalBlue{225, 105, 65};
  inline static const cv::Scalar kSaddleBrown{19, 69, 139};
  inline static const cv::Scalar kSalmon{114, 128, 250};
  inline static const cv::Scalar kSandyBrown{96, 164, 244};
  inline static const cv::Scalar kSeaGreen{87, 139, 46};
  inline static const cv::Scalar kSeaShell{238, 245, 255};
  inline static const cv::Scalar kSienna{45, 82, 160};
  inline static const cv::Scalar kSilver{192, 192, 192};
  inline static const cv::Scalar kSkyBlue{235, 206, 135};
  inline static const cv::Scalar kSlateBlue{205, 90, 106};
  inline static const cv::Scalar kSlateGray{144, 128, 112};
  inline static const cv::Scalar kSlateGrey{144, 128, 112};
  inline static const cv::Scalar kSnow{250, 250, 255};
  inline static const cv::Scalar kSpringGreen{127, 255, 0};
  inline static const cv::Scalar kSteelBlue{180, 130, 70};
  inline static const cv::Scalar kTan{140, 180, 210};
  inline static const cv::Scalar kTeal{128, 128, 0};
  inline static const cv::Scalar kThistle{216, 191, 216};
  inline static const cv::Scalar kTomato{71, 99, 255};
  inline static const cv::Scalar kTurquoise{208, 224, 64};
  inline static const cv::Scalar kViolet{238, 130, 238};
  inline static const cv::Scalar kWheat{179, 222, 245};
  inline static const cv::Scalar kWhite{255, 255, 255};
  inline static const cv::Scalar kWhiteSmoke{245, 245, 245};
  inline static const cv::Scalar kYellow{0, 255, 255};
  inline static const cv::Scalar kYellowGreen{50, 205, 154};
};

}  // namespace aa::shared
