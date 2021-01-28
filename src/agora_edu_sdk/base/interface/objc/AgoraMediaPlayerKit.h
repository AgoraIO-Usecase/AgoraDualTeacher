//
//  AgoraMediaPlayerKit.h
//  AgoraMediaPlayer
//
//  Copyright © 2019 agora. All rights reserved.
//

#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>
#import <VideoToolbox/VideoToolbox.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
typedef UIView View;
#elif TARGET_OS_MAC
#import <AppKit/AppKit.h>
typedef NSView View;
#endif
NS_ASSUME_NONNULL_BEGIN
/** AgoraMediaPlayerState, reporting the playback state. */
typedef NS_ENUM(NSInteger, AgoraMediaPlayerState) {
  /** `0`: Default state. */
  AgoraMediaPlayerStateIdle = 0,
  /** `1`: Opening the media resource. */
  AgoraMediaPlayerStateOpening = 1,
  /** `2`: Opened the media resource successfully. */
  AgoraMediaPlayerStateOpenCompleted = 2,
  /** `3`: Playing the media resource. */
  AgoraMediaPlayerStatePlaying = 3,
  /** `4`: Pauses the playback. */
  AgoraMediaPlayerStatePaused = 4,
  /** `5`: The playback is completed. */
  AgoraMediaPlayerStatePlayBackCompleted = 5,
  /** `6`: Stops the playback. */
  AgoraMediaPlayerStateStopped = 6,
  /** `100`: Fails to play the media resource. */
  AgoraMediaPlayerStateFailed = 100,
};
/** AgoraMediaPlayerError, reporting the player's error code. */
typedef NS_ENUM(NSInteger, AgoraMediaPlayerError) {
  /** `0`: No error. */
  AgoraMediaPlayerErrorNone = 0,
  /** `-1`: Invalid arguments. */
  AgoraMediaPlayerErrorInvalidArguments = -1,
  /** `-2`: Internal error. */
  AgoraMediaPlayerErrorInternal = -2,
  /** `-3`: No resource. */
  AgoraMediaPlayerErrorNoSource = -3,
  /** `-4`: Invalid media resource. */
  AgoraMediaPlayerErrorInvalidMediaSource = -4,
  /** `-5`: The type of the media stream is unknown. */
  AgoraMediaPlayerErrorUnknowStreamType = -5,
  /** `-6`: The object is not initialized. */
  AgoraMediaPlayerErrorObjNotInitialized = -6,
  /** `-7`: The codec is not supported. */
  AgoraMediaPlayerErrorCodecNotSupported = -7,
  /** `-8`: Invalid renderer. */
  AgoraMediaPlayerErrorVideoRenderFailed = -8,
  /** `-9`: Error occurs in the internal state of the player. */
  AgoraMediaPlayerErrorInvalidState = -9,
  /** `-10`: The URL of the media resource can not be found. */
  AgoraMediaPlayerErrorUrlNotFound = -10,
  /** `-11`: Invalid connection between the player and Agora's Server. */
  AgoraMediaPlayerErrorInvalidConnectState = -11,
  /** `-12`: The playback buffer is insufficient. */
  AgoraMediaPlayerErrorSrcBufferUnderflow = -12,
};
/** AgoraMediaPlayerEvent, reporting the result of the seek operation to the new 
 playback position.
 */
typedef NS_ENUM(NSInteger, AgoraMediaPlayerEvent) {
  /** `0`: Begins to seek to the new playback position. */
  AgoraMediaPlayerEventSeekBegin = 0,
  /** `1`: Finish seeking to the new playback position. */
  AgoraMediaPlayerEventSeekComplete = 1,
  /** `2`: Error occurs when seeking to the new playback position. */
  AgoraMediaPlayerEventSeekError = 2,
};

/**
 * AgoraMediaPlayerMetaDataType, reporting the type of the media metadata.
 */
typedef NS_ENUM(NSUInteger, AgoraMediaPlayerMetaDataType) {
  /** `0`: The type is unknown. */
  AgoraMediaPlayerMetaDataTypeUnknown = 0,
  /** `1`: The type is SEI. */
  AgoraMediaPlayerMetaDataTypeSEI = 1,
};

/** AgoraMediaPixelFormat, reporting the pixel format of the video stream. */
typedef NS_ENUM(NSInteger, AgoraMediaPixelFormat) {
  /** `0`: The format is known.
   */
  AgoraMediaPixelFormatUnknown = 0,
  /** `1`: The format is I420. 
   */
  AgoraMediaPixelFormatI420 = 1,
  /** `2`: The format is BGRA.
   */
  AgoraMediaPixelFormatBGRA = 2,
  /** `3`: The format is Planar YUV422.
   */
  AgoraMediaPixelFormatI422 = 3,
  /** `8`: The format is NV12. 
   */
  AgoraMediaPixelFormatNV12 = 8,
};
/** AgoraMediaStreamType, reporting the type of the media stream. */
typedef NS_ENUM(NSInteger, AgoraMediaStreamType) {
  /** `0`: The type is unknown. */
  AgoraMediaStreamTypeUnknow = 0,
  /** `1`: The video stream.  */
  AgoraMediaStreamTypeVideo = 1,
  /** `2`: The audio stream. */
  AgoraMediaStreamTypeAudio = 2,
  /** `3`: The subtitle stream. */
  AgoraMediaStreamTypeSubtitle = 3,
};
/** AgoraMediaPlayerRenderMode, reporting the render mode of the player. */
typedef NS_ENUM(NSUInteger, AgoraMediaPlayerRenderMode) {
    /** `1`: Uniformly scale the video until it fills the visible boundaries 
     (cropped). One dimension of the video may have clipped contents.
     */
    AgoraMediaPlayerRenderModeHidden = 1,

    /** `2`: Uniformly scale the video until one of its dimension fits the 
     boundary (zoomed to fit). Areas that are not filled due to disparity in 
     the aspect ratio are filled with black.
     */
    AgoraMediaPlayerRenderModeFit = 2,
};
@class AgoraMediaPlayer;

@class AgoraMediaStreamInfo;
/** The AgoraMediaPlayerDelegate protocol, reporting the event callbacks. */
@protocol AgoraMediaPlayerDelegate <NSObject>

@optional

/** Reports the playback state change.

 @param playerKit AgoraMediaPlayer

 @param state The new playback state after change. See AgoraMediaPlayerState.

 @param error The player's error code. See AgoraMediaPlayerError.
 */
- (void)AgoraMediaPlayer:(AgoraMediaPlayer *_Nonnull)playerKit
       didChangedToState:(AgoraMediaPlayerState)state
                   error:(AgoraMediaPlayerError)error;

/** Reports current playback progress.
   
 The callback occurs once every one second during the playback and reports 
 current playback progress.

 @param playerKit AgoraMediaPlayer

 @param position Current playback progress (s).
 */
- (void)AgoraMediaPlayer:(AgoraMediaPlayer *_Nonnull)playerKit
    didChangedToPosition:(NSInteger)position;

/** Reports the result of the seek operation.

 @param playerKit AgoraMediaPlayer

 @param event The result of the seek operation. See AgoraMediaPlayerEvent.
 */
- (void)AgoraMediaPlayer:(AgoraMediaPlayer *_Nonnull)playerKit
          didOccurEvent:(AgoraMediaPlayerEvent)event;

/** Reports the reception of the media metadata.

 The callback occurs when the player receivers the media metadata and reports
 the detailed information of the media metadata.

 @param playerKit AgoraMediaPlayer

 @param type The type of the media metadata. See AgoraMediaPlayerMetaDataType.

 @param data The detailed data of the media metadata.

 @param length The length (byte) of the data.
 */
- (void)AgoraMediaPlayer:(AgoraMediaPlayer *_Nonnull)playerKit
            metaDataType:(AgoraMediaPlayerMetaDataType) type
          didReceiveData:(NSString *)data
                  length:(NSInteger)length;

/**
  @brief Triggered when media file are played once
 */
- (void)AgoraMediaPlayerOnCompleted:(AgoraMediaPlayer *_Nonnull)playerKit;

/** Occurs when each time the player receives a video frame.

 After registering the video frame observer, the callback occurs when each
 time the player receives a video frame, reporting the detailed 
 information of the video frame.

 @param playerKit AgoraMediaPlayer

 @param pixelBuffer The detailed information of the video frame. 
 */
- (void)AgoraMediaPlayer:(AgoraMediaPlayer *_Nonnull)playerKit
    didReceiveVideoFrame:(CVPixelBufferRef)pixelBuffer;
/** Occurs when each time the player receives an audio frame.

 After registering the audio frame observer, the callback occurs when each
 time the player receives an audio frame, reporting the detailed 
 information of the audio frame.

 @param playerKit AgoraMediaPlayer

 @param audioFrame The detailed information of the audio frame.
 */
- (void)AgoraMediaPlayer:(AgoraMediaPlayer *_Nonnull)playerKit
    didReceiveAudioFrame:(CMSampleBufferRef)audioFrame;

@end

/** The AgoraMediaStreamInfo class, reporting the whole detailed information of
 the media stream. 
 */
__attribute__((visibility("default"))) @interface AgoraMediaStreamInfo : NSObject
/** The index of the media stream. */
@property(nonatomic, assign) NSInteger streamIndex;
/** The type of the media stream. See AgoraMediaStreamType for details. */
@property(nonatomic, assign) AgoraMediaStreamType streamType;
/** The codec of the media stream. */
@property(nonatomic, copy) NSString *_Nonnull codecName;
/** The language of the media stream. */
@property(nonatomic, copy) NSString *_Nullable language;
/** For video stream, gets the frame rate (fps). */
@property(nonatomic, assign) NSInteger videoFrameRate;
/** For video stream, gets the bitrate (bps). */
@property(nonatomic, assign) NSInteger videoBitRate;
/** For video stream, gets the width (pixel) of the video. */
@property(nonatomic, assign) NSInteger videoWidth;
/** For video stream, gets the height (pixel) of the video. */
@property(nonatomic, assign) NSInteger videoHeight;
/** For the audio stream, gets the sample rate (Hz). */
@property(nonatomic, assign) NSInteger audioSampleRate;
/** For the audio stream, gets the channel number. */
@property(nonatomic, assign) NSInteger audioChannels;
/** The total duration (s) of the media stream. */
@property(nonatomic, assign) NSInteger duration;
/** The rotation of the video stream. */
@property(nonatomic, assign) NSInteger rotation;

@end

/** The AgoraMediaPlayer interface. */
__attribute__((visibility("default"))) @interface AgoraMediaPlayer : NSObject
/** Sets whether to mute the media resource:

* YES: Mute the media resource.
* NO: Unmute the media resource.
*/
@property(nonatomic, assign) BOOL mute;
/** The local playback volume, which ranges from 0 to 100. 0: Mute. 100: (default) The original volume. */
@property(nonatomic, assign) NSInteger volume;
/** The playback state. See AgoraMediaPlayerState */
@property(nonatomic, readonly) AgoraMediaPlayerState state;
@property(nonatomic, weak) id<AgoraMediaPlayerDelegate> _Nullable delegate;
/** Creates an AgoraMediaPlayer instance.

 @param delegate AgoraMediaPlayerDelegate

 @return The AgoraMediaPlayer instance.
 */
- (instancetype)initWithDelegate:(id<AgoraMediaPlayerDelegate>)delegate;

/** Sets the player's render view.

 @param view The player's render view.
 */
- (void)setView:(View *_Nullable)view;

/** Sets the player's render view.

 @param mode The player's render view. See AgoraMediaPlayerRenderMode.
 */
- (void)setRenderMode:(AgoraMediaPlayerRenderMode)mode;

/** Opens the media resource.

 @param url The path of the media resource. Both local path and online path 
 are supported.

 @param startPos The starting position (s) for playback. Default value is 0.
 */
- (void)open:(NSString *)url startPos:(NSInteger)startPos;

/** Plays the media resource.

 After open the media resource or pause the playback, you can call this 
 method to play the media resource.
 */
- (void)play;

/** Pauses the playback.

 To resume the playback, call the play method.
 */
- (void)pause;

/** Stops the playback. 
 */
- (void)stop;

/** Seeks to a new playback position.

 After successfully calling the method, you will receive the didOccurEvent 
 callback, reporting the result of the seek operation to the new playback 
 position.
 @param position The new playback position (s).
 */
- (void)seekToPosition:(NSInteger)position;

/** Sets whether to mute the media resource.
 @param isMute Sets whether to mute the media resource:

 * YES: Mute the media resource.
 * NO: (default) Unmute the media resource.

 @return * 0: Success.
 * < 0: Failure. See AgoraMediaPlayerError.
 */
- (int)mute:(bool)isMute;

/** Confirms whether the media resource is muted.
 @return * If the call succeeds, returns:
  - YES: The media resource is muted.
  - NO: (default) The media resource is not muted.
 * If the call fails, returns NO.
 */
- (bool)getMute;

/** Adjusts the local playback volume.
 @param volume The local playback volume, which ranges from 0 to 100: * 0: Mute
 * 100: (default) The original volume.
 @return * 0: Success
 * < 0: Failure. See AgoraMediaPlayerError.
 */
- (int)adjustVolume:(int)volume;

/** Gets current local playback progress.
 @return * < 0: Failure. See AgoraMediaPlayerError.
 * Others: The call succeeds and returns current playback progress (s).
 */
- (NSInteger)getPosition;

/** Gets the duration of the media resource.
 @return * < 0: Failure. See AgoraMediaPlayerError.
 * Others: The call succeeds and returns the total duration of the media 
 resource (s).
 */
- (NSInteger)getDuration;

/** Gets current playback state.

 @return * The call succeeds and returns current playback state. See 
 AgoraMediaPlayerState.
 * The call fails and returns nil.
 */
- (AgoraMediaPlayerState)getPlayerState;

/** Gets the number of the media streams in the media resource.

 @return * < 0: Failure. See AgoraMediaPlayerError.
 * Others: The call succeeds and returns the number of the media streams in
 the media resource.
 */
- (NSInteger)getStreamCount;

/** Gets the detailed information of the media stream.

 @param index The index of the media stream.

 @return * If the call succeeds, returns the detailed information of the media
   stream. See AgoraMediaStreamInfo.
 * If the call fails and returns nil.
 */
- (AgoraMediaStreamInfo *_Nullable)getStreamByIndex:(int)index;


@end

NS_ASSUME_NONNULL_END
