import cv2

def play_avi(avi_file):
    # 打开AVI文件
    video = cv2.VideoCapture(avi_file)

    while True:
        # 读取视频帧
        ret, frame = video.read()

        if not ret:
            # 视频读取完毕或无法读取帧时退出循环
            break

        # 显示视频帧
        cv2.imshow('AVI Player', frame)

        # 按下'q'键退出播放
        if cv2.waitKey(25) & 0xFF == ord('q'):
            break

    # 释放资源
    video.release()
    cv2.destroyAllWindows()

# 使用示例
if __name__ == "__main__":
    print("Hello")
    avi_file = './example.avi'
    play_avi(avi_file)