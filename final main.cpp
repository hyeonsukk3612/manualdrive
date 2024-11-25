#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {
    VideoCapture source("7_lt_ccw_100rpm_in.mp4"); // ���� ���� ����
    if (!source.isOpened()) { cerr << "Failed to open video file!" << endl;return -1;}// ���� ������ ������ �ʾ��� ���
    Mat frame, gray, binary; // ������, �׷��̽�����, ����ȭ �̹����� ���� Mat ��ü ����
    Point mainPoint; // �� ������ ��ǥ
    bool isFirstFrame = true; // ù ������ ���� Ȯ�� �÷���

    while (true) {
        source >> frame; // �������� ������ �б�
        if (frame.empty()) break; // �������� ��������� ���� ����
        cvtColor(frame, gray, COLOR_BGR2GRAY); // �÷� �̹����� �׷��̽����Ϸ� ��ȯ
        gray += Scalar(100) - mean(gray); // ��� ����
        threshold(gray, binary, 128, 255, THRESH_BINARY); // ����ȭ ó��
        vector<Rect> objects; // ��ü �簢�� ���� ����
        vector<Point> centers; // ��ü �߽��� ���� ����
        Mat label, stats, centroids, output; // ���� ��� �м��� ��� �̹����� ���� Mat ��ü ����
        Rect roi(0, max(0, binary.rows - 90), binary.cols, 90); // ���� ����(ROI) ����
        binary = binary(roi); // ROI ����
        if (isFirstFrame) { // ù �������� ���
            mainPoint = Point(binary.cols / 2, binary.rows - 1); // �� ������ �ʱ�ȭ
            isFirstFrame = false; // ù ������ �÷��� ����
        }
        int num = connectedComponentsWithStats(binary, label, stats, centroids); // ���� ��� �м� ����
        int closestIndex = -1; // ���� ����� ��ü�� �ε���
        int minDist = binary.cols; // �ּ� �Ÿ� �ʱ�ȭ
        for (int i = 1; i < num; i++) { // ��� ���� ��ҿ� ���� �ݺ� (��� ����)
            int area = stats.at<int>(i, CC_STAT_AREA); // ��ü ���� ���
            if (area > 120) { // ���� ũ�� �̻��� ��ü�� ó��
                Point center(cvRound(centroids.at<double>(i, 0)), cvRound(centroids.at<double>(i, 1))); // �߽��� ���
                Rect object(stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP),
                stats.at<int>(i, CC_STAT_WIDTH), stats.at<int>(i, CC_STAT_HEIGHT)); // ��ü �簢�� ����
                objects.push_back(object); // ��ü �簢�� ����
                centers.push_back(center); // �߽��� ����
                int dist = norm(center - mainPoint); // �� ���������� �Ÿ� ���
                if (dist <= 140 && dist < minDist) { // ���� ����� ��ü ã��
                    minDist = dist;
                    closestIndex = objects.size() - 1;
                }
            }
        }
        cvtColor(binary, output, COLOR_GRAY2BGR); // ���� �̹����� �÷��� ��ȯ
        for (size_t i = 0; i < objects.size(); i++) { // ��� ��ü�� ���� �ݺ�
            Scalar color = (i == closestIndex) ? Scalar(0, 0, 255) : Scalar(255, 0, 0); // ���� ���� (���� ����� ��ü�� ������)
            rectangle(output, objects[i], color, 2); // ��ü �簢�� �׸���
            circle(output, centers[i], 5, color, -1); // ��ü �߽��� �׸���
        }
        if (closestIndex >= 0) mainPoint = centers[closestIndex]; // ���� ����� ��ü�� ���� ��� / �� ������ ������Ʈ
        int error = (output.cols / 2) - mainPoint.x; // ȭ�� �߾Ӱ� �� �������� x��ǥ ���� ���
        cout << "error: " << error << endl; // ���� ���
        imshow("Frame", frame); // ���� ������ ǥ��
        imshow("Detection", output); // ó���� �̹��� ǥ��
        if (waitKey(30) == 'q') break; // 'q' Ű�� ������ ���� ����
    }
    return 0;// �����ϰ� �Լ� ����
}
