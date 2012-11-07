#include "DTW2.h"
#include "Visualizer.h"

using namespace std;
using namespace cv;

DTW2::DTW2(void)
{
}

DTW2::DTW2(int n, int m)
:	n_(n), 
	m_(m),
	window_(int( Maximum(n, m) / 10.0 ))
{
	grid_ = new float* [n];

	for(int i = 0; i < n; i++)
		grid_[i] = new float[m];

	dtwImg_ = Mat(Size(n, m), CV_64FC1, FLT_MAX );
}

DTW2::~DTW2(void)
{
	for(int i = 0; i < n_; i++)
		delete[] grid_[i];

	delete[] grid_;
}

float DTW2::FastDTW(vector<double> &v, vector<double> &w, int distType, bool showGrid) 
{
	for (int i = 0; i < n_; i++)
		for (int j = 0; j < m_; j++)
			if(distType == EUCLIDEAN_DST)
				dtwImg_.at<float>(j, i) = grid_[i][j] = EuclideanDistance(v[i], w[j]);
			else if(distType == L1_DST)
				dtwImg_.at<float>(j, i) = grid_[i][j] = L1Distance(v[i], w[j]);
			else
				dtwImg_.at<float>(j, i) = grid_[i][j] = FLT_MAX;


	for (int i = 1; i < n_; i++) 
		for(int j = (int)Maximum(1.0f, i - window_); j < (int)Minimum(m_, i + window_ + 1.0f); j++) 
			if(distType == EUCLIDEAN_DST)
				dtwImg_.at<float>(j, i) = grid_[i][j] = EuclideanDistance(v[i], w[j]) + Minimum(grid_[i-1][j], Minimum(grid_[i][j-1], grid_[i-1][j-1]));
			else if(distType == L1_DST)
				dtwImg_.at<float>(j, i) = grid_[i][j] = L1Distance(v[i], w[j]) + Minimum(grid_[i-1][j], Minimum(grid_[i][j-1], grid_[i-1][j-1]));
			else
				dtwImg_.at<float>(j, i) = grid_[i][j] = FLT_MAX;

			if(showGrid)
			{	
				Mat dtwImg8;

				dtwImg_.convertTo(dtwImg8, CV_8U);
				resize(dtwImg8, dtwImg8, Size( 320.0, dtwImg8.rows * (320.0 / dtwImg8.cols) ) );

				char text[100];
				sprintf_s(text, sizeof(text) - 1, "DTW (%s) distance: %.2lf", distType == EUCLIDEAN_DST ? "Euclidean" : "L1",  grid_[n_ - 1][m_ - 1]);

				putText(dtwImg8, text, Point(10, 20), CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(0), 2);
				putText(dtwImg8, text, Point(10, 20), CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(255), 1);

				VisualizerPtr->ShowImage("DTW Grid - Old version", dtwImg8);
			}

			return grid_[n_ - 1][m_ - 1];
}

float DTW2::L1Distance(double p1, double p2) 
{
	return float(fabs(p1 - p2));
	//return fabs(pt1.x - pt2.x) + fabs(pt1.y - pt2.y);
}

float DTW2::EuclideanDistance(double p1, double p2) 
{
	return float((p1 - p2) * (p1 - p2));
	//return sqrt(pow(pt1.x - pt2.x, 2.0f) + pow(pt1.y - pt2.y, 2.0f));
}

double DTW2::DtwSeq(const vector<double>& seq1, const vector<double>& seq2, double& err)
{
    err = 0;

    if (seq1.size() < 5 || seq2.size() < 5)
        return -1.0;

    int m = seq1.size();
    int n = seq2.size();

    int minSize = m;
    if (minSize > n)
        minSize = n;

    vector<double> tmpseq1;
    vector<double> tmpseq2;

    if (minSize == m)
    {
        tmpseq1 = seq1;
        float q = (float)n / float(m);
        for (int i = 0; i < m; i++)
        {
            int t = i * q;
            tmpseq2.push_back(seq2[t]);
        }
    }
    else
    {
        tmpseq2 = seq2;
        float q = (float)m / (float)n;
        for (int i = 0; i < n; i++)
        {
            int t = i * q;
            tmpseq1.push_back(seq1[t]);
        }
    }


    vector<vector <double> > cost(minSize, minSize);

    cost[0][0] = GetDistanceGr(tmpseq1[0], tmpseq2[0], 0);

    for (int i = 1; i < minSize; i++)
        cost[i][0] = cost[i - 1][0] + GetDistanceGr(tmpseq1[i], tmpseq2[0], i);


    for (int j = 1; j < minSize; j++)
        cost[0][j] = cost[0][j - 1] + GetDistanceGr(tmpseq1[0], tmpseq2[j], j);


    for (int j = 1; j < minSize; j++)
        for (int i = 1; i < minSize; i++)
            cost[i][j] = min(cost[i-1][j], min(cost[i][j-1], cost[i-1][j-1])) +  GetDistanceGr(tmpseq1[i], tmpseq2[j], i - j);


    dtwMap = Mat(minSize, minSize, CV_8UC3, Scalar(255,255,255));
    line(dtwMap, Point(0, 0), Point(minSize - 1, minSize - 1), Scalar(128, 128, 128), 1, 8, 0);

    int i = minSize - 1;
    int j = minSize - 1;

    double length = 0.0f;
    double d = 1.0f;
    double plt = 1;
    double fVal = 0;
    while ( (i + j) != 0)
    {
        dtwMap.at<Vec3b>(j,i)[0] = 0;
        dtwMap.at<Vec3b>(j,i)[1] = 0;
        dtwMap.at<Vec3b>(j,i)[2] = 0;
        d = 1;
        double minval;
        if (i == 0) 
        {
            j -= 1;	
            minval = cost[i][j];
        } 
        else 
            if (j == 0) 
            {
                i -= 1;
                minval = cost[i][j];
            } 
            else
            {
                double a = cost[i - 1][j];
                double b = cost[i][j - 1];
                double c = cost[i - 1][j - 1];

                minval = min(a, min(b, c));

                if (minval == a)
                    if (a / c > 0.95)
                    {
                        minval = c;
                        plt++;
                    }

                    if (minval == b)
                        if (b / c > 0.95)
                        {
                            minval = c;
                            plt++;
                        }

                        if (minval == a)
                            i -= 1;
                        else
                            if (minval == b)
                                j -= 1;
                            else
                            {
                                i -= 1;
                                j -= 1;
                                d = 1.4142;
                            }


            }
            length += d;

            fVal += minval;

    }


    err = ((length - sqrt(pow(minSize, 2.0) + pow(minSize, 2.0))) / length) * 100;
    VisualizerPtr->ShowImage("GS", dtwMap);
    return cost[minSize - 1][minSize - 1] / length;

    //	return fVal / length;

}

double DTW2::GetDistanceGr(double p1, double p2, double pd)
{
    //	return sqrt(pow(p1 - p2, 2) + pow(pd, 2));
    return abs(p1 - p2);

}
