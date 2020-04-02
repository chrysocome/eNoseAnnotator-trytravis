#include "mvector.h"
#include "measurementdata.h"

#include <QtCore>


/*!
  \class MVector
  \brief The MVector class represents one measurement vector of the eNose sensor.
  \ingroup classes
  The size measurement values received every two seconds from the eNose sensor can be stored in MVectors.
  MVector can contain absolute resistance values, but also deviations relative to a base resistance (R0).
*/
MVector::MVector(int size):
    size(size)
{
    // init array
    for (int i=0; i<size; i++)
        vector.push_back(0.0);
}

MVector::~MVector()
{

}

QString MVector::toString()
{
    QStringList stringList;

    for (int i=0; i<size; i++)
        stringList << QString::number(vector[i]);

    stringList << userAnnotation.toString() << detectedAnnotation.toString();

    return stringList.join(";");
}

bool MVector::operator==(const MVector &other) const
{
    Q_ASSERT(other.size == this->size);

    double epsilon = 0.0001;

    // this == other, if ||this->array[i]-other.array[i]|| < epsilon for i<size
    for (int i=0; i<size; i++)
        if (qAbs(this->vector[i]-other.vector[i]) > epsilon)
            return false;

    return true;
}

bool MVector::operator!=(const MVector &other) const
{    
    // not equal
    return !this->operator==(other);
}

MVector MVector::operator*(const double multiplier)
{
    MVector vector;

    for (int i=0; i<size; i++)
        vector[i] = this->vector[i] * multiplier;

    return vector;
}

MVector MVector::operator*(const int multiplier)
{
    return *this * static_cast<double>(multiplier);
}

MVector MVector::operator/(const double denominator)
{
    MVector vector;

    for (int i=0; i<size; i++)
        vector[i] = this->vector[i] / denominator;

    return vector;
}

MVector MVector::operator/(const int denominator)
{
    return *this / static_cast<double>(denominator);
}

MVector MVector::operator+(const MVector other)
{
    Q_ASSERT(other.size == this->size);

    MVector vector;

    for (int i=0; i<size; i++)
        vector[i] = this->vector[i] + other.vector[i];

    return vector;
}

double &MVector::operator[](int index)
{
    Q_ASSERT("index out of range!" && index >= 0 && index < size);

    vector.at(index);
    return vector[index];
}

/*!
 returns a MVector with zero in each component.
 */
MVector MVector::zeroes()
{
    return MVector();
}

/*!
   returns a MVector based on this relative to baseVector.
   baseVector is expected to be absolute.
 */
MVector MVector::getRelativeVector(MVector baseVector)
{
    Q_ASSERT(baseVector.size == this->size);

    // cp vector data
    MVector deviationVector = *this;

    // calculate deviation / %
    for (int i=0; i<size; i++)
    {
        deviationVector[i] = 100 * ((this->vector[i] /  baseVector[i]) - 1.0);
    }

    return deviationVector;
}

/*!
   returns an absolute MVector based on this, which is relative to baseVector.
   baseVector is expected to be absolute.
 */
MVector MVector::getAbsoluteVector(MVector baseVector)
{
    Q_ASSERT(baseVector.size == this->size);

    // cp vector data
    MVector absoluteVector = *this;

    // calculate absolute resistances / Ohm
    for (int i=0; i<size; i++)
    {
        absoluteVector[i] = ((this->vector[i] / 100.0) + 1.0) * baseVector[i];
    }
    return absoluteVector;
}

/*!
 * \brief MVector::getFuncVector return MVector of functionalisation averages
 * \param functionalisation
 * \param sensorFailures
 * \return
 */
MVector MVector::getFuncVector(std::array<int, MVector::nChannels> functionalisation, std::array<bool, MVector::nChannels> sensorFailures)
{
    Q_ASSERT(functionalisation.size() == this->size);
    Q_ASSERT(sensorFailures.size() == this->size);

    // get func map
    auto funcMap = MeasurementData::getFuncMap(functionalisation, sensorFailures);

    // init func vector
    auto keyList = funcMap.keys();
    int maxFunc = *std::max_element(keyList.begin(), keyList.end());
    MVector funcVector(maxFunc+1);

    // copy atributes
    funcVector.userAnnotation = userAnnotation;
    funcVector.detectedAnnotation = detectedAnnotation;

    // calc averages of functionalisations
    for (int i=0; i<MVector::nChannels; i++)
    {
        if (!sensorFailures[i])
        {
            int func = functionalisation[i];
            funcVector[func] += vector[i] / funcMap[func];
        }
    }

    return funcVector;
}
