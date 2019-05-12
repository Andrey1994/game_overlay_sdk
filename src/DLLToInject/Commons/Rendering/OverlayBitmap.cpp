//
// Copyright(c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "OverlayBitmap.h"
#include "Utility/MessageLog.h"

#include "../Recording/RecordingState.h"
#include "Utility/StringUtils.h"

#include <sstream>

using namespace Microsoft::WRL;

const D2D1_COLOR_F OverlayBitmap::clearColor_ = { 0.0f, 0.0f, 0.0f, 0.01f };
const D2D1_COLOR_F OverlayBitmap::fpsBackgroundColor_ = { 0.0f, 0.0f, 0.0f, 0.8f };
const D2D1_COLOR_F OverlayBitmap::msBackgroundColor_ = { 0.0f, 0.0f, 0.0f, 0.7f };
const D2D1_COLOR_F OverlayBitmap::messageBackgroundColor_ = { 0.0f, 0.0f, 0.0f, 0.5f };
const D2D1_COLOR_F OverlayBitmap::fontColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };
const D2D1_COLOR_F OverlayBitmap::numberColor_ = { 1.0f, 162.0f / 255.0f, 26.0f / 255.0f, 1.0f };
const D2D1_COLOR_F OverlayBitmap::recordingColor_ = { 1.0f, 0.0f, 0.0f, 1.0f };

const D2D1_COLOR_F OverlayBitmap::colorBarSequence_[] = {
    {1.0f, 1.0f, 1.0f, 1.0f},                                   // White
    {0.0f, 1.0f, 0.0f, 1.0f},                                   // Lime
    {0.0f, 102.0f / 255.0f, 1.0f, 1.0f},                        // Blue
    {1.0f, 0.0f, 0.0f, 1.0f},                                   // Red
    {204.0f / 255.0f, 204.0f / 255.0f, 1.0f, 1.0f},             // Teal
    {51.0f / 255.0f, 204.0f / 255.0f, 1.0f, 1.0f},              // Navy
    {0.0f, 176.0f / 255.0f, 0.0f, 1.0f},                        // Green
    {0.0f, 1.0f, 1.0f, 1.0f},                                   // Aqua
    {138.0f / 255.0f, 135.0f / 255.0f, 0.0f, 1.0f},             // Dark green
    {221.0f / 255.0f, 221.0f / 255.0f, 221.0f / 255.0f, 1.0f},  // Silver
    {153.0f / 255.0f, 0.0f, 204.0f / 255.0f, 1.0f},             // Purple
    {185.0f / 255.0f, 172.0f / 255.0f, 3.0f / 255.0f, 1.0f},    // Olive
    {119.0f / 255.0f, 119.0f / 255.0f, 119.0f / 255.0f, 1.0f},  // Gray
    {128.0f / 255.0f, 0.0f, 128.0f / 255.0f, 1.0f},             // Fuchsia
    {1.0f, 255.0f / 255.0f, 0.0f, 1.0f},                        // Yellow
    {1.0f, 153.0f / 255.0f, 0.0f, 1.0f},                        // Orange
};

OverlayBitmap::RawData::RawData() : dataPtr{ nullptr }, size{ 0 }
{
    // Empty
}

OverlayBitmap::OverlayBitmap()
{
    // Empty
}

bool OverlayBitmap::Init(int screenWidth, int screenHeight, API api)
{
    if (!InitFactories()) {
        return false;
    }

    CalcSize(screenWidth, screenHeight);

    if (!InitBitmap()) {
        return false;
    }

    if (!InitText()) {
        return false;
    }

    return true;
}

OverlayBitmap::~OverlayBitmap()
{
    message_.reset();

    renderTarget_.Reset();
    textBrush_.Reset();
    helperLineBrush_.Reset();
    textFormat_.Reset();
    messageFormat_.Reset();
    iwicFactory_.Reset();
    bitmap_.Reset();
    bitmapLock_.Reset();
    writeFactory_.Reset();
    d2dFactory_.Reset();

    if (coInitialized_) {
        CoUninitialize();
        coInitialized_ = false;
    }
}

void OverlayBitmap::CalcSize(int screenWidth, int screenHeight)
{
    fullWidth_ = 256;
    fullHeight_ = lineHeight_ * 8;
    messageHeight_ = lineHeight_ * 3;

    Resize(screenWidth, screenHeight);

    const auto fullWidth = static_cast<FLOAT>(fullWidth_);
    const auto fullHeight = static_cast<FLOAT>(fullHeight_);
    const auto lineHeight = static_cast<FLOAT>(lineHeight_);
    const auto halfWidth = static_cast<FLOAT>(fullWidth_ / 2);
    const auto messageHeight = static_cast<FLOAT>(messageHeight_);

    const auto barHeight = static_cast<FLOAT>(screenHeight);
    int barWidth = 24;
    const int indexUpperLeft = static_cast<int>(Alignment::UpperLeft);
    messageArea_ = D2D1::RectF(0.0f, barHeight - fullHeight_, barWidth + fullWidth, barHeight);

    fullArea_.d2d1 = D2D1::RectF(0.0f, 0.0f, fullWidth + barWidth, barHeight);
    fullArea_.wic = {0, 0, fullWidth_ + barWidth, screenHeight_ };
}

void OverlayBitmap::Resize(int screenWidth, int screenHeight)
{
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
    UpdateScreenPosition();
}

void OverlayBitmap::UpdateScreenPosition()
{
    screenPosition_.x = 0;
    screenPosition_.y = 0;
}

void OverlayBitmap::DrawOverlay()
{
    StartRendering();
    Update();
    FinishRendering();
}

void OverlayBitmap::StartRendering()
{
    renderTarget_->BeginDraw();
    renderTarget_->SetTransform(D2D1::IdentityMatrix());
}

void OverlayBitmap::Update()
{
    const auto textureState = RecordingState::GetInstance().Update();
    UpdateScreenPosition();
    renderTarget_->Clear(clearColor_);  // clear full bitmap
    if (RecordingState::GetInstance().IsOverlayShowing()) {
        DrawMessage();
    }
}

void OverlayBitmap::DrawMessage()
{
    renderTarget_->PushAxisAlignedClip(messageArea_, D2D1_ANTIALIAS_MODE_ALIASED);
    renderTarget_->Clear(messageBackgroundColor_);
    char *message = RecordingState::GetInstance().GetOverlayMessage();
    std::string str_message(message);
    std::wstring wstr_message = ConvertUTF8StringToUTF16String(str_message);
    message_->WriteMessage(wstr_message.c_str());
    message_->SetText(writeFactory_.Get(), textFormat_.Get());
    message_->Draw(renderTarget_.Get());

    renderTarget_->PopAxisAlignedClip();
}

void OverlayBitmap::FinishRendering()
{
    HRESULT hr = renderTarget_->EndDraw();
    if (FAILED(hr)) {
        g_messageLog.LogWarning("OverlayBitmap", "EndDraw failed, HRESULT", hr);
    }
}

OverlayBitmap::RawData OverlayBitmap::GetBitmapDataRead()
{
    if (bitmapLock_) {
        g_messageLog.LogWarning("OverlayBitmap", "Bitmap lock was not released");
    }
    bitmapLock_.Reset();

    RawData rawData = {};
    const auto& currBitmapArea = fullArea_.wic;
    HRESULT hr = bitmap_->Lock(&currBitmapArea, WICBitmapLockRead, &bitmapLock_);
    if (FAILED(hr)) {
        g_messageLog.LogWarning("OverlayBitmap", "Bitmap lock failed, HRESULT", hr);
        return rawData;
    }

    hr = bitmapLock_->GetDataPointer(&rawData.size, &rawData.dataPtr);
    if (FAILED(hr)) {
        g_messageLog.LogWarning("OverlayBitmap", "Bitmap lock GetDataPointer failed, HRESULT", hr);
        rawData = {};
    }
    return rawData;
}

void OverlayBitmap::UnlockBitmapData() { bitmapLock_.Reset(); }

int OverlayBitmap::GetFullWidth() const { return fullWidth_ * 2; }

int OverlayBitmap::GetFullHeight() const { return screenHeight_; }

OverlayBitmap::Position OverlayBitmap::GetScreenPos() const { return screenPosition_; }

const D2D1_RECT_F& OverlayBitmap::GetCopyArea() const { return fullArea_.d2d1; }

VkFormat OverlayBitmap::GetVKFormat() const { return VK_FORMAT_B8G8R8A8_UNORM; }

bool OverlayBitmap::InitFactories()
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&d2dFactory_));
    if (FAILED(hr)) {
        g_messageLog.LogError("OverlayBitmap", "D2D1CreateFactory failed, HRESULT", hr);
        return false;
    }

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(writeFactory_),
        reinterpret_cast<IUnknown**>(writeFactory_.GetAddressOf()));
    if (FAILED(hr)) {
        g_messageLog.LogError("OverlayBitmap", "DWriteCreateFactory failed, HRESULT", hr);
        return false;
    }

    hr = CoInitialize(NULL);
    if (hr == S_OK || hr == S_FALSE) {
        coInitialized_ = true;
    }
    else {
        g_messageLog.LogWarning("OverlayBitmap", "CoInitialize failed, HRESULT", hr);
    }

    hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&iwicFactory_));
    if (FAILED(hr)) {
        g_messageLog.LogError("OverlayBitmap", "CoCreateInstance failed, HRESULT", hr);
        return false;
    }
    return true;
}

bool OverlayBitmap::InitBitmap()
{
    HRESULT hr = iwicFactory_->CreateBitmap(
        fullWidth_ * 2, screenHeight_, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &bitmap_);
    if (FAILED(hr)) {
        g_messageLog.LogError("OverlayBitmap", "CreateBitmap failed, HRESULT", hr);
        return false;
    }

    const auto rtProperties = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 0.0f, 0.0f,
        D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE);

    hr = d2dFactory_->CreateWicBitmapRenderTarget(bitmap_.Get(), rtProperties, &renderTarget_);
    if (FAILED(hr)) {
        g_messageLog.LogError("OverlayBitmap", "CreateWicBitmapRenderTarget failed, HRESULT", hr);
        return false;
    }

    hr = renderTarget_->CreateSolidColorBrush(fontColor_, &textBrush_);
    if (FAILED(hr)) {
        g_messageLog.LogError("OverlayBitmap", "CreateTextFormat failed, HRESULT", hr);
        return false;
    }

    hr = renderTarget_->CreateSolidColorBrush(numberColor_, &helperLineBrush_);
    if (FAILED(hr)) {
        g_messageLog.LogError("OverlayBitmap", "CreateTextFormat failed, HRESULT", hr);
        return false;
    }

    return true;
}

bool OverlayBitmap::InitText()
{
    textFormat_ =
        CreateTextFormat(25.0f, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    messageFormat_ =
        CreateTextFormat(20.0f, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    message_.reset(new TextMessage(renderTarget_.Get(), fontColor_, numberColor_));
    message_->SetArea(messageArea_.left, messageArea_.top, messageArea_.right - messageArea_.left,
        messageArea_.bottom - messageArea_.top);
    return true;
}

IDWriteTextFormat* OverlayBitmap::CreateTextFormat(float size, DWRITE_TEXT_ALIGNMENT textAlignment,
    DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment)
{
    IDWriteTextFormat* textFormat;
    HRESULT hr = writeFactory_->CreateTextFormat(L"Verdane", NULL, DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        size, L"en-us", &textFormat);
    if (FAILED(hr)) {
        g_messageLog.LogError("OverlayBitmap", "CreateTextFormat failed, HRESULT", hr);
        return false;
    }
    textFormat->SetTextAlignment(textAlignment);
    textFormat->SetParagraphAlignment(paragraphAlignment);
    return textFormat;
}
