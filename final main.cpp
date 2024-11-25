#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {
    VideoCapture source("7_lt_ccw_100rpm_in.mp4"); // 비디오 파일 열기
    if (!source.isOpened()) { cerr << "Failed to open video file!" << endl;return -1;}// 비디오 파일이 열리지 않았을 경우
    Mat frame, gray, binary; // 프레임, 그레이스케일, 이진화 이미지를 위한 Mat 객체 선언
    Point mainPoint; // 주 관심점 좌표
    bool isFirstFrame = true; // 첫 프레임 여부 확인 플래그

    while (true) {
        source >> frame; // 비디오에서 프레임 읽기
        if (frame.empty()) break; // 프레임이 비어있으면 루프 종료
        cvtColor(frame, gray, COLOR_BGR2GRAY); // 컬러 이미지를 그레이스케일로 변환
        gray += Scalar(100) - mean(gray); // 밝기 조정
        threshold(gray, binary, 128, 255, THRESH_BINARY); // 이진화 처리
        vector<Rect> objects; // 객체 사각형 저장 벡터
        vector<Point> centers; // 객체 중심점 저장 벡터
        Mat label, stats, centroids, output; // 연결 요소 분석과 출력 이미지을 위한 Mat 객체 선언
        Rect roi(0, max(0, binary.rows - 90), binary.cols, 90); // 관심 영역(ROI) 설정
        binary = binary(roi); // ROI 추출
        if (isFirstFrame) { // 첫 프레임일 경우
            mainPoint = Point(binary.cols / 2, binary.rows - 1); // 주 관심점 초기화
            isFirstFrame = false; // 첫 프레임 플래그 해제
        }
        int num = connectedComponentsWithStats(binary, label, stats, centroids); // 연결 요소 분석 수행
        int closestIndex = -1; // 가장 가까운 객체의 인덱스
        int minDist = binary.cols; // 최소 거리 초기화
        for (int i = 1; i < num; i++) { // 모든 연결 요소에 대해 반복 (배경 제외)
            int area = stats.at<int>(i, CC_STAT_AREA); // 객체 면적 계산
            if (area > 120) { // 일정 크기 이상의 객체만 처리
                Point center(cvRound(centroids.at<double>(i, 0)), cvRound(centroids.at<double>(i, 1))); // 중심점 계산
                Rect object(stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP),
                stats.at<int>(i, CC_STAT_WIDTH), stats.at<int>(i, CC_STAT_HEIGHT)); // 객체 사각형 생성
                objects.push_back(object); // 객체 사각형 저장
                centers.push_back(center); // 중심점 저장
                int dist = norm(center - mainPoint); // 주 관심점과의 거리 계산
                if (dist <= 140 && dist < minDist) { // 가장 가까운 객체 찾기
                    minDist = dist;
                    closestIndex = objects.size() - 1;
                }
            }
        }
        cvtColor(binary, output, COLOR_GRAY2BGR); // 이진 이미지를 컬러로 변환
        for (size_t i = 0; i < objects.size(); i++) { // 모든 객체에 대해 반복
            Scalar color = (i == closestIndex) ? Scalar(0, 0, 255) : Scalar(255, 0, 0); // 색상 설정 (가장 가까운 객체는 빨간색)
            rectangle(output, objects[i], color, 2); // 객체 사각형 그리기
            circle(output, centers[i], 5, color, -1); // 객체 중심점 그리기
        }
        if (closestIndex >= 0) mainPoint = centers[closestIndex]; // 가장 가까운 객체가 있을 경우 / 주 관심점 업데이트
        int error = (output.cols / 2) - mainPoint.x; // 화면 중앙과 주 관심점의 x좌표 차이 계산
        cout << "error: " << error << endl; // 오차 출력
        imshow("Frame", frame); // 원본 프레임 표시
        imshow("Detection", output); // 처리된 이미지 표시
        if (waitKey(30) == 'q') break; // 'q' 키를 누르면 루프 종료
    }
    return 0;// 리턴하고 함수 종료
}
