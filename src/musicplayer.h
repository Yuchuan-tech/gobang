#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

// 初始化背景音乐播放器（自动循环）
// 返回: 1=成功, 0=失败
int InitBackgroundMusic(const char* musicPath);

// 每帧调用
void UpdateBackgroundMusic(void);

// 设置音量 (0.0 ~ 1.0)
void SetBackgroundMusicVolume(float volume);

// 暂停/恢复
void PauseBackgroundMusic(void);
void ResumeBackgroundMusic(void);

// 释放资源
void CloseBackgroundMusic(void);

#endif