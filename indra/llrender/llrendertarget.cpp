/**
 * @file llrendertarget.cpp
 * @brief LLRenderTarget implementation
 *
 * $LicenseInfo:firstyear=2001&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "linden_common.h"

#include "llrendertarget.h"
#include "llrender.h"
#include "llgl.h"

LLRenderTarget* LLRenderTarget::sBoundTarget = NULL;
U32 LLRenderTarget::sBytesAllocated = 0;

void check_framebuffer_status()
{
    if (gDebugGL)
    {
        GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        switch (status)
        {
        case GL_FRAMEBUFFER_COMPLETE:
            break;
        default:
            LL_WARNS() << "check_framebuffer_status failed -- " << std::hex << status << LL_ENDL;
            ll_fail("check_framebuffer_status failed");
            break;
        }
    }
}

bool LLRenderTarget::sUseFBO = false;
U32 LLRenderTarget::sCurFBO = 0;


extern S32 gGLViewport[4];

U32 LLRenderTarget::sCurResX = 0;
U32 LLRenderTarget::sCurResY = 0;

LLRenderTarget::LLRenderTarget() :
    mResX(0),
    mResY(0),
    mFBO(0),
    mDepth(0),
    mUseDepth(false),
    mUsage(LLTexUnit::TT_TEXTURE)
{
}

LLRenderTarget::~LLRenderTarget()
{
    release();
}

void LLRenderTarget::resize(U32 resx, U32 resy)
{
    //for accounting, get the number of pixels added/subtracted
    S32 pix_diff = (resx*resy)-(mResX*mResY);

    mResX = resx;
    mResY = resy;

    llassert(mInternalFormat.size() == mTex.size());

    for (U32 i = 0; i < mTex.size(); ++i)
    { //resize color attachments
        gGL.getTexUnit(0)->bindManual(mUsage, mTex[i]);
        LLImageGL::setManualImage(LLTexUnit::getInternalType(mUsage), 0, mInternalFormat[i], mResX, mResY, GL_RGBA, GL_UNSIGNED_BYTE, NULL, false);
        sBytesAllocated += pix_diff*4;
    }

    if (mDepth)
    {
        gGL.getTexUnit(0)->bindManual(mUsage, mDepth);
        U32 internal_type = LLTexUnit::getInternalType(mUsage);
        LLImageGL::setManualImage(internal_type, 0, GL_DEPTH_COMPONENT24, mResX, mResY, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL, false);

        sBytesAllocated += pix_diff*4;
    }
}


bool LLRenderTarget::allocate(U32 resx, U32 resy, U32 color_fmt, bool depth, LLTexUnit::eTextureType usage, LLTexUnit::eTextureMipGeneration generateMipMaps)
{
    LL_PROFILE_ZONE_SCOPED_CATEGORY_DISPLAY;
    llassert(usage == LLTexUnit::TT_TEXTURE);
    llassert(!isBoundInStack());

    resx = llmin(resx, (U32) gGLManager.mGLMaxTextureSize);
    resy = llmin(resy, (U32) gGLManager.mGLMaxTextureSize);

    release();

    mResX = resx;
    mResY = resy;

    mUsage = usage;
    mUseDepth = depth;

    mGenerateMipMaps = generateMipMaps;

    if (mGenerateMipMaps != LLTexUnit::TMG_NONE) {
        // Calculate the number of mip levels based upon resolution that we should have.
        mMipLevels = 1 + (U32)floor(log10((float)llmax(mResX, mResY)) / log10(2.0));
    }

    if (depth)
    {
        if (!allocateDepth())
        {
            LL_WARNS() << "Failed to allocate depth buffer for render target." << LL_ENDL;
            return false;
        }
    }

    glGenFramebuffers(1, (GLuint *) &mFBO);

    if (mDepth)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, LLTexUnit::getInternalType(mUsage), mDepth, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, sCurFBO);
    }

    return addColorAttachment(color_fmt);
}

void LLRenderTarget::setColorAttachment(LLImageGL* img, LLGLuint use_name)
{
    LL_PROFILE_ZONE_SCOPED;
    llassert(img != nullptr); // img must not be null
    llassert(sUseFBO); // FBO support must be enabled
    llassert(mDepth == 0); // depth buffers not supported with this mode
    llassert(mTex.empty()); // mTex must be empty with this mode (binding target should be done via LLImageGL)
    llassert(!isBoundInStack());

    U32 target = getTarget();

    if (mFBO == 0)
    {
        glGenFramebuffers(1, (GLuint*)&mFBO);
    }

    mResX = img->getWidth();
    mResY = img->getHeight();
    mUsage = img->getTarget();

    if (use_name == 0)
    {
        use_name = img->getTexName();
    }

    mTex.push_back(use_name);

    glBindFramebuffer(target, mFBO);
    glFramebufferTexture2D(target, GL_COLOR_ATTACHMENT0,
            LLTexUnit::getInternalType(mUsage), use_name, 0);
        stop_glerror();

    check_framebuffer_status();

    glBindFramebuffer(target, sCurFBO);
}

void LLRenderTarget::releaseColorAttachment()
{
    LL_PROFILE_ZONE_SCOPED;
    llassert(!isBoundInStack());
    llassert(mTex.size() == 1); //cannot use releaseColorAttachment with LLRenderTarget managed color targets
    llassert(mFBO != 0);  // mFBO must be valid

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, LLTexUnit::getInternalType(mUsage), 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, sCurFBO);
    LOG_GLERROR(mName + " releaseColorAttachment()");

    mTex.clear();
}

bool LLRenderTarget::addColorAttachment(U32 color_fmt)
{
    LL_PROFILE_ZONE_SCOPED_CATEGORY_DISPLAY;
    llassert(!isBoundInStack());

    if (color_fmt == 0)
    {
        return true;
    }

    U32 target = getTarget();

    U32 offset = static_cast<U32>(mTex.size());

    if( offset >= 4 )
    {
        LL_WARNS() << "Too many color attachments" << LL_ENDL;
        llassert( offset < 4 );
        return false;
    }
    if( offset > 0 && (mFBO == 0) )
    {
        llassert(  mFBO != 0 );
        return false;
    }

    U32 tex;
    LLImageGL::generateTextures(1, &tex);
    gGL.getTexUnit(0)->bindManual(mUsage, tex);

    U32 miplevel = mMipLevels;
    U32 intformat = color_fmt;
    U32 pixformat = GL_RGBA;
    U32 pixtype = GL_UNSIGNED_BYTE;

    {
        //clear_glerror();

        if(color_fmt == GL_RGB10_A2)
        {
            pixformat = GL_RGBA;
            pixtype = GL_UNSIGNED_BYTE;
        }
        else if(color_fmt == GL_R11F_G11F_B10F)
        {
            pixformat = GL_RGB;
            pixtype = GL_HALF_FLOAT;
        }
        else if(color_fmt == GL_R8)
        {
            pixformat = GL_RED;
            pixtype = GL_UNSIGNED_BYTE;
        }
        else if(color_fmt == GL_R16F)
        {
            pixformat = GL_RED;
            pixtype = GL_HALF_FLOAT;
        }
        else if(color_fmt ==  GL_RG16F)
        {
            pixformat = GL_RG;
            pixtype = GL_HALF_FLOAT;
        }
        else if(color_fmt == GL_RGBA16F)
        {
            pixformat = GL_RGBA;
            pixtype = GL_HALF_FLOAT;
        }
        else if(color_fmt == GL_RGB16F)
        {
            pixformat = GL_RGB;
            pixtype = GL_HALF_FLOAT;
        }
        else if(color_fmt == GL_RGB8)
        {
            pixformat = GL_RGB;
            pixtype = GL_UNSIGNED_BYTE;
        }
        else if(color_fmt == GL_RGB)
        {
            pixformat = GL_RGB;
            pixtype = GL_UNSIGNED_BYTE;
        }
        else if(color_fmt == GL_RGBA)
        {
            pixformat = GL_RGBA;
            pixtype = GL_UNSIGNED_BYTE;
        }
        else if(color_fmt == GL_RGBA8)
        {
            pixformat = GL_RGBA;
            pixtype = GL_UNSIGNED_BYTE;
        }
        else
        {
            pixformat = GL_RGBA;
            pixtype = GL_UNSIGNED_BYTE;
        }

        LLImageGL::setManualImage(LLTexUnit::getInternalType(mUsage), 0, color_fmt, mResX, mResY, pixformat, pixtype, NULL, false);
        if (glGetError() != GL_NO_ERROR)
        {
            LL_WARNS() << "Could not allocate color buffer for render target." << LL_ENDL;
            return false;
        }
    }

    sBytesAllocated += mResX*mResY*4;

    if (offset == 0)
    { //use bilinear filtering on single texture render targets that aren't multisampled
        gGL.getTexUnit(0)->setTextureFilteringOption(LLTexUnit::TFO_BILINEAR);
        LOG_GLERROR(mName + " setting filtering to TFO_BILINEAR");
    }
    else
    { //don't filter data attachments
        gGL.getTexUnit(0)->setTextureFilteringOption(LLTexUnit::TFO_POINT);
        LOG_GLERROR(mName + " setting filtering to TFO_POINT");
    }

#if GL_VERSION_3_1
    if (mUsage != LLTexUnit::TT_RECT_TEXTURE)
    {
        gGL.getTexUnit(0)->setTextureAddressMode(LLTexUnit::TAM_MIRROR);
        LOG_GLERROR(mName + " setting address mode to TAM_MIRROR");
    }
    else
#endif
    {
        // ATI doesn't support mirrored repeat for rectangular textures.
        gGL.getTexUnit(0)->setTextureAddressMode(LLTexUnit::TAM_CLAMP);
        LOG_GLERROR(mName + " setting address mode to TAM_CLAMP");
    }

    if (mFBO)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+offset,
            LLTexUnit::getInternalType(mUsage), tex, 0);

        check_framebuffer_status();

        glBindFramebuffer(GL_FRAMEBUFFER, sCurFBO);
    }

    mTex.push_back(tex);
    mInternalFormat.push_back(color_fmt);

    if (gDebugGL)
    { //bind and unbind to validate target
        bindTarget(mName, mMode);
        flush();
    }


    return true;
}

bool LLRenderTarget::allocateDepth()
{
    LL_PROFILE_ZONE_SCOPED_CATEGORY_DISPLAY;
    LLImageGL::generateTextures(1, &mDepth);
    gGL.getTexUnit(0)->bindManual(mUsage, mDepth);

    U32 internal_type = LLTexUnit::getInternalType(mUsage);
    LLImageGL::setManualImage(internal_type, 0, GL_DEPTH_COMPONENT24, mResX, mResY, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL, false);
    gGL.getTexUnit(0)->setTextureFilteringOption(LLTexUnit::TFO_POINT);

    sBytesAllocated += mResX*mResY*4;

    if (glGetError() != GL_NO_ERROR)
    {
        LL_WARNS() << "Unable to allocate depth buffer for render target " << mName << LL_ENDL;
        return false;
    }

    return true;
}

void LLRenderTarget::shareDepthBuffer(LLRenderTarget& target)
{
    llassert(!isBoundInStack());

    if (!mFBO || !target.mFBO)
    {
        LL_ERRS() << "Cannot share depth buffer between non FBO render targets." << LL_ENDL;
    }

    if (target.mDepth)
    {
        LL_ERRS() << "Attempting to override existing depth buffer.  Detach existing buffer first." << LL_ENDL;
    }

    if (target.mUseDepth)
    {
        LL_ERRS() << "Attempting to override existing shared depth buffer. Detach existing buffer first." << LL_ENDL;
    }

    if (mDepth)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, target.mFBO);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, LLTexUnit::getInternalType(mUsage), mDepth, 0);

        check_framebuffer_status();

        glBindFramebuffer(GL_FRAMEBUFFER, sCurFBO);

        LOG_GLERROR(mName + " shareDepthBuffer()");

        target.mUseDepth = true;
    }
}

void LLRenderTarget::release()
{
    LL_PROFILE_ZONE_SCOPED_CATEGORY_DISPLAY;
    llassert(!isBoundInStack());

    if (mDepth)
    {
        LLImageGL::deleteTextures(1, &mDepth);

        mDepth = 0;

        sBytesAllocated -= mResX*mResY*4;
    }
    else if (mFBO)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

        if (mUseDepth)
        { //detach shared depth buffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, LLTexUnit::getInternalType(mUsage), 0, 0);
            mUseDepth = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, sCurFBO);
    }

    // Detach any extra color buffers (e.g. SRGB spec buffers)
    //
    if (mFBO && (mTex.size() > 1))
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        size_t z;
        for (z = mTex.size() - 1; z >= 1; z--)
        {
            sBytesAllocated -= mResX*mResY*4;
            glFramebufferTexture2D(GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0+z), LLTexUnit::getInternalType(mUsage), 0, 0);
            LLImageGL::deleteTextures(1, &mTex[z]);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, sCurFBO);
    }

    if (mFBO)
    {
        if (mFBO == sCurFBO)
        {
            sCurFBO = 0;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glDeleteFramebuffers(1, (GLuint *) &mFBO);
        mFBO = 0;
    }

    if (mTex.size() > 0)
    {
        sBytesAllocated -= mResX*mResY*4;
        LLImageGL::deleteTextures(1, &mTex[0]);
    }

    LOG_GLERROR(mName + " release()");

    mTex.clear();
    mInternalFormat.clear();

    mResX = mResY = 0;
}

void LLRenderTarget::bindTarget(std::string name_, U32 mode_)
{
    LL_PROFILE_GPU_ZONE("bindTarget");
    llassert(mFBO);
    llassert(!isBoundInStack());

    //mode_ = 0;

    mMode = mode_;
    mName = name_;

    U32 target = getTarget();

    glBindFramebuffer(target, mFBO);
    LOG_GLERROR(mName+" bindTarget()");

    //attach();

    sCurFBO = mFBO;

    //setup multiple render targets
    GLenum drawbuffers[] = {GL_COLOR_ATTACHMENT0,
                            GL_COLOR_ATTACHMENT1,
                            GL_COLOR_ATTACHMENT2,
                            GL_COLOR_ATTACHMENT3};

    if (mTex.empty())
    { //no color buffer to draw to
        if(!mUseDepth) LL_WARNS() << mName << " HAS NO COLOR BUFFER AND NO DEPTH!!" << LL_ENDL;
        GLenum buffers[] = {GL_NONE};
        glDrawBuffers(0, buffers);
        glReadBuffer(GL_NONE);
    }
    else if(mMode == 0)
    {
        glDrawBuffers(static_cast<GLsizei>(mTex.size()), drawbuffers);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        LOG_GLERROR(mName+" read and write buffers");
    }
    else if(mMode == 1)
    {
        glDrawBuffers(static_cast<GLsizei>(mTex.size()), drawbuffers);
        glReadBuffer(GL_NONE);
        LOG_GLERROR(mName+" draw buffer");
    }
    else if(mMode == 2)
    {
        GLenum buffers[] = {GL_NONE};
        glDrawBuffers(0, buffers);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        LOG_GLERROR(mName+" read buffer");
    }

    check_framebuffer_status();
    LOG_GLERROR(mName+" checked status");

    glViewport(0, 0, mResX, mResY);
    sCurResX = mResX;
    sCurResY = mResY;

    mPreviousRT = sBoundTarget;
    sBoundTarget = this;
}

void LLRenderTarget::clear(U32 mask_in)
{
    LL_PROFILE_GPU_ZONE("clear");
    llassert(mFBO);
    U32 mask = 0;

    if(!mTex.empty()) mask |= GL_COLOR_BUFFER_BIT;
    if (mUseDepth)
    {
        mask |= GL_DEPTH_BUFFER_BIT;

    }
    if (mFBO)
    {
        check_framebuffer_status();
        glClear(mask & mask_in);
        LOG_GLERROR(mName + "clear()");
    }
    else
    {
        LLGLEnable scissor(GL_SCISSOR_TEST);
        glScissor(0, 0, mResX, mResY);
        LOG_GLERROR("");
        glClear(mask & mask_in);
    }
}

U32 LLRenderTarget::getTexture(U32 attachment) const
{
    if (attachment >= mTex.size())
    {
        LL_WARNS() << "Invalid attachment index " << attachment << " for size " << mTex.size() << LL_ENDL;
        llassert(false);
        return 0;
    }
    return mTex[attachment];
}

U32 LLRenderTarget::getNumTextures() const
{
    return static_cast<U32>(mTex.size());
}

void LLRenderTarget::bindTexture(U32 index, S32 channel, LLTexUnit::eTextureFilterOptions filter_options)
{
    gGL.getTexUnit(channel)->bindManual(mUsage, getTexture(index), filter_options == LLTexUnit::TFO_TRILINEAR || filter_options == LLTexUnit::TFO_ANISOTROPIC);

    bool isSRGB = false;
    llassert(mInternalFormat.size() > index);
    switch (mInternalFormat[index])
    {
        case GL_SRGB:
        case GL_SRGB8:
#if GL_VERSION_2_1
        case GL_SRGB_ALPHA:
#endif
        case GL_SRGB8_ALPHA8:
            isSRGB = true;
            break;

        default:
            break;
    }

    gGL.getTexUnit(channel)->setTextureFilteringOption(filter_options);

    LOG_GLERROR(mName + " bindTexture()");
}

void LLRenderTarget::flush()
{
    LL_PROFILE_GPU_ZONE("rt flush");

    LOG_GLERROR(mName+" rt flush() A");

    gGL.flush();
    llassert(mFBO);
    llassert(sCurFBO == mFBO);
    llassert(sBoundTarget == this);

    if (mGenerateMipMaps == LLTexUnit::TMG_AUTO && mMode != 2)
    {
        LL_PROFILE_GPU_ZONE("rt generate mipmaps");
        //bindTexture(0, 0, LLTexUnit::TFO_TRILINEAR);
        bindTexture(0, 0, LLTexUnit::TFO_ANISOTROPIC);
        glGenerateMipmap(GL_TEXTURE_2D);
        LOG_GLERROR(mName + " glGenerateMipmap()");
    }

    LOG_GLERROR(mName + " rt flush() B");

    unbind();
}

void LLRenderTarget::unbind()
{
    if (mPreviousRT)
    {
        // a bit hacky -- pop the RT stack back two frames and push
        // the previous frame back on to play nice with the GL state machine
        sBoundTarget = mPreviousRT->mPreviousRT;
        mPreviousRT->bindTarget(mPreviousRT->mName, mPreviousRT->mMode);
    }
    else
    {
        sBoundTarget = nullptr;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        sCurFBO = 0;
        glViewport(gGLViewport[0], gGLViewport[1], gGLViewport[2], gGLViewport[3]);
        sCurResX = gGLViewport[2];
        sCurResY = gGLViewport[3];
        glReadBuffer(GL_BACK);
        glDrawBuffer(GL_BACK);
    }

    LOG_GLERROR(mName + " rt unbind()");
}

bool LLRenderTarget::isComplete() const
{
    return !mTex.empty() || mDepth;
}

void LLRenderTarget::getViewport(S32* viewport)
{
    viewport[0] = 0;
    viewport[1] = 0;
    viewport[2] = mResX;
    viewport[3] = mResY;
}

bool LLRenderTarget::isBoundInStack() const
{
    LLRenderTarget* cur = sBoundTarget;
    while (cur && cur != this)
    {
        cur = cur->mPreviousRT;
    }

    return cur == this;
}

void LLRenderTarget::swapFBORefs(LLRenderTarget& other)
{
    // Must be initialized
    llassert(mFBO);
    llassert(other.mFBO);

    // Must be unbound
    // *NOTE: mPreviousRT can be non-null even if this target is unbound - presumably for debugging purposes?
    llassert(sCurFBO != mFBO);
    llassert(sCurFBO != other.mFBO);
    llassert(!isBoundInStack());
    llassert(!other.isBoundInStack());

    // Must be same type
    llassert(sUseFBO == other.sUseFBO);
    llassert(mResX == other.mResX);
    llassert(mResY == other.mResY);
    llassert(mInternalFormat == other.mInternalFormat);
    llassert(mTex.size() == other.mTex.size());
    llassert(mDepth == other.mDepth);
    llassert(mUseDepth == other.mUseDepth);
    llassert(mGenerateMipMaps == other.mGenerateMipMaps);
    llassert(mMipLevels == other.mMipLevels);
    llassert(mUsage == other.mUsage);

    std::swap(mFBO, other.mFBO);
    std::swap(mTex, other.mTex);
}

U32 LLRenderTarget::getTarget()
{
    U32 target = GL_FRAMEBUFFER;

    if(mMode == 1) target = GL_DRAW_FRAMEBUFFER;
    else if(mMode == 2) target = GL_READ_FRAMEBUFFER;

    return target;
}
