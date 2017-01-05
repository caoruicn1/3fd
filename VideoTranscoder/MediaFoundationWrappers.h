#ifndef MEDIAFOUNDATIONWRAPPERS_H // header guard
#define MEDIAFOUNDATIONWRAPPERS_H

#include "3FD\base.h"
#include <string>
#include <chrono>
#include <mutex>
#include <map>
#include <wrl.h>
#include <d3d11.h>
#include <mfreadwrite.h>

namespace application
{
    using std::string;

    using namespace Microsoft::WRL;

    /// <summary>
    /// Uses RAII to initialize and finalize Microsoft Media Foundation library.
    /// </summary>
    class MediaFoundationLib
    {
    public:

        MediaFoundationLib();
        ~MediaFoundationLib();
    };

    const char *TranslateMFTCategory(const GUID &mftCategory);

    ComPtr<ID3D11Device> GetDeviceDirect3D(UINT idxVideoAdapter);

    /// <summary>
    /// Holds the most important information about a video stream.
    /// </summary>
    struct VideoProperties
    {
        GUID videoDecFormat;
        UINT32 videoEncAvgBitRate;
        UINT32 videoInterlaceMode;
        UINT32 videoWidth;
        UINT32 videoHeigth;
        MFRatio videoFPS;
        
        std::chrono::microseconds GetVideoFrameDuration() const
        {
            return std::chrono::microseconds((long long)(videoFPS.Denominator * 1e6 / videoFPS.Numerator));
        };
    };

    /// <summary>
    /// Wraps Media Foundation Source Reader object.
    /// </summary>
    class MFSourceReader : notcopiable
    {
    private:

        DWORD m_streamCount;
        ComPtr<IMFSourceReader> m_mfSourceReader;
        ComPtr<IMFSourceReaderCallback> m_srcReadCallback;

        void ConfigureDecoderTransforms(bool mustReconfigAll);

    public:

        MFSourceReader(const string &url, const ComPtr<IMFDXGIDeviceManager> &mfDXGIDevMan);

        void GetMediaPropertiesFrom(DWORD idxStream,
                                    std::map<DWORD, VideoProperties> &videoProps,
                                    std::chrono::microseconds &duration) const;

        void ReadSampleAsync();

        enum ReadStateFlags
        {
            EndOfStream        = MF_SOURCE_READERF_ENDOFSTREAM,
            NewStreamAvailable = MF_SOURCE_READERF_NEWSTREAM,
            NativeTypeChanged  = MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED,
            GapFound           = MF_SOURCE_READERF_STREAMTICK
        };

        ComPtr<IMFSample> GetSample(DWORD &state);
    };
    /*
    /// <summary>
    /// Wraps Media Foundation Sink Writer object.
    /// </summary>
    class MFSinkWriter : notcopiable
    {
    private:

        ComPtr<IMFSinkWriter> m_mfSinkWriter;
        std::map<DWORD, uint64_t> m_gapTrackByStreamIdx;
        std::mutex m_gapTrackAccessMutex;

        //void ConfigureEncoderTransforms(bool mustReconfigAll);

    public:

        MFSinkWriter(const string &url,
                     const std::map<DWORD, VideoProperties> &videoProps,
                     const ComPtr<IMFDXGIDeviceManager> &mfDXGIDevMan);

        void WriteSampleAsync();

        void SendStreamGapAsync();

        void Finalize();
    };*/

}// end of namespace application

#endif // end of header guard
