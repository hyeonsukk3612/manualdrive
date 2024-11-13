#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "dxl.hpp"
#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

bool save_video = false;
void save_signal_handler(int)
{
    save_video = true;
}

int main(void)
{
    // 이미지 전송
    string src = "nvarguscamerasrc sensor-id=0 ! \
    video/x-raw(memory:NVMM), width=(int)640, height=(int)360, \
    format=(string)NV12, framerate=(fraction)30/1 ! \
    nvvidconv flip-method=0 ! video/x-raw, \
    width=(int)640, height=(int)360, format=(string)BGRx ! \
    videoconvert ! video/x-raw, format=(string)BGR ! appsink";

    VideoCapture source(src, CAP_GSTREAMER);
    if (!source.isOpened()){ cout << "카메라 오류" << endl; return -1; }

    string dst1 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! \
    nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! \
    h264parse ! rtph264pay pt=96 ! \
    udpsink host=203.234.58.168 port=8001 sync=false";

    VideoWriter writer1(dst1,0, (double)30,Size(640,360),true);
    if(!writer1.isOpened()) {cerr<<"작성기 열기 실패!"<<endl; return -1;}
    Mat frame, gray;

    // 영상 저장을 위한 VideoWriter
    VideoWriter video_saver;
    vector<Mat> frames; // 프레임을 저장할 벡터

    // dynamixel control
    Dxl mx;
    struct timeval start,end1;
    double diff1;
    int vel1=0,vel2=0;
    int goal1=0,goal2=0;

    signal(SIGTSTP, save_signal_handler); // Ctrl+Z 시그널 핸들러 (Ctrl+S 대신 사용)
    if(!mx.open()) { cout << "dxl open error"<<endl; return -1; } //장치열기

    while(true)
    {
        gettimeofday(&start,NULL);
        
        source >> frame;
        if (frame.empty()){ cerr << "frame empty!" << endl; break; }
        writer1 << frame; //이미지 전송
        frames.push_back(frame.clone()); // 프레임 저장
                
        if (mx.kbhit())
        {
            char c = mx.getch();
            switch(c)
            {
            case 's': goal1 = 0; goal2 = 0; break;
            case ' ': goal1 = 0; goal2 = 0; break;
            case 'f': goal1 = 50; goal2 = -50; break;
            case 'b': goal1 = -50; goal2 = 50; break;
            case 'l': goal1 = -50; goal2 = -50; break;
            case 'r': goal1 = 50; goal2 = 50; break;
            default : goal1 = 0; goal2 = 0; break;
            }         
        }
        
        // 가속 및 감속 모션 생성
        if(goal1>vel1) vel1 += 5;
        else if(goal1<vel1) vel1 -= 5;
        else vel1 = goal1;

        if(goal2>vel2) vel2 += 5;
        else if(goal2<vel2) vel2 -= 5;
        else vel2 = goal2;

        if(!mx.setVelocity(vel1,vel2)){ cout << "setVelocity error"<<endl; return -1;}
                
        if (save_video) break; // Ctrl+Z가 눌렸을 때 루프 종료
        usleep(20*1000);        
        gettimeofday(&end1,NULL);
        diff1 = end1.tv_sec + end1.tv_usec / 1000000.0 - start.tv_sec - start.tv_usec / 1000000.0;
        cout <<"vel1:" << vel1 <<','<<"vel2:"<< vel2 << ",time:" << diff1 << endl;
    }   
    
    // Ctrl+Z가 눌렸을 때 영상 저장
    if (save_video && !frames.empty()) {
        cout << "영상 저장 중..." << endl;
        string filename = "captured_video.avi";
        video_saver.open(filename, VideoWriter::fourcc('M','J','P','G'), 30, frames[0].size());
        
        if (!video_saver.isOpened()) {
            cerr << "영상 파일을 열 수 없습니다." << endl;
        } else {
            for (const auto& frame : frames) {
                video_saver.write(frame);
            }
            cout << filename << "에 영상이 저장되었습니다." << endl;
        }
        video_saver.release();
    }

    mx.close();
    return 0;
}
