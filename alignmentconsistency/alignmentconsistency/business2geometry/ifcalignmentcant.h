#pragma once


#include "generic.h"

#include "ifcaxis2placement3d.h"
#include "ifccosine.h"
#include "ifcline.h"
#include "ifcproductdefinitionshape.h"
#include "ifcsecondorderpolynomialspiral.h"
#include "ifcseventhorderpolynomialspiral.h"
#include "ifcsine.h"
#include "ifcthirdorderpolynomialspiral.h"


static  inline  int_t   ___SegmentCount__alignmentCant(
                                int_t   ifcCantAlignmentInstance
                            )
{
	int_t	* aggrSegments = nullptr;

    sdaiGetAttrBN(ifcCantAlignmentInstance, "Segments", sdaiAGGR, &aggrSegments);

    return  sdaiGetMemberCount(aggrSegments);
}

static  inline  int_t   ___CreateSegmentedReferenceCurve__alignmentCant(
                                int_t   model,
                                int_t   ifcCantAlignmentInstance,
                                double  startDistAlongHorizontalAlignment
                            )
{
    if (ifcCantAlignmentInstance == 0) {
        return  0;
    }

    double  railHeadDistance = 0.;
    sdaiGetAttrBN(ifcCantAlignmentInstance, "RailHeadDistance", sdaiREAL, &railHeadDistance);
    if (railHeadDistance == 0.) {
        assert(false);
        railHeadDistance = 1.;
    }

    int_t	ifcSegmentedReferenceCurveInstance = sdaiCreateInstanceBN(model, "IFCSEGMENTEDREFERENCECURVE"),
            * aggrCurveSegment = sdaiCreateAggrBN(ifcSegmentedReferenceCurveInstance, "Segments"),
        	* aggrSegments = nullptr;

    char    selfIntersect[2] = "F";
    sdaiPutAttrBN(ifcSegmentedReferenceCurveInstance, "SelfIntersect", sdaiENUM, (void*) selfIntersect);

    int_t   noSegmentInstances =
                ___GetAlignmentSegments(
                        model,
                        ifcCantAlignmentInstance,
                        nullptr
                    );

    if (noSegmentInstances) {
        int_t   * segmentInstances = new int_t[noSegmentInstances];

        ___GetAlignmentSegments(
                model,
                ifcCantAlignmentInstance,
                segmentInstances
            );

        for (int_t i = 0; i < noSegmentInstances; i++) {
            int_t   ifcAlignmentSegmentInstance = segmentInstances[i];
            assert(sdaiGetInstanceType(ifcAlignmentSegmentInstance) == sdaiGetEntity(model, "IFCALIGNMENTSEGMENT"));

            int_t   ifcAlignmentCantSegmentInstance = 0;
            sdaiGetAttrBN(ifcAlignmentSegmentInstance, "DesignParameters", sdaiINSTANCE, (void*) &ifcAlignmentCantSegmentInstance);
            assert(sdaiGetInstanceType(ifcAlignmentCantSegmentInstance) == sdaiGetEntity(model, "IFCALIGNMENTCANTSEGMENT"));

            {
                int_t   ifcCurveSegmentInstance = sdaiCreateInstanceBN(model, "IFCCURVESEGMENT");

                //
                //  Add geometry for Ifc...Alignment...
                //
                sdaiPutAttrBN(
                        ifcAlignmentSegmentInstance,
                        "ObjectPlacement",
                        sdaiINSTANCE,
                        (void*) ___CreateObjectPlacement(
                                        model
                                    )
                    );
                sdaiPutAttrBN(
                        ifcAlignmentSegmentInstance,
                        "Representation",
                        sdaiINSTANCE,
                        (void*) ___CreateProductDefinitionShape(
                                        model,
                                        ifcCurveSegmentInstance,
                                        false
                                    )
                    );

                //
                //  ENTITY IfcAlignmentCantSegment
                //      StartDistAlong      IfcLengthMeasure
                //      HorizontalLength    IfcPositiveLengthMeasure
                //      StartCantLeft       IfcLengthMeasure
                //      EndCantLeft         OPTIONAL IfcLengthMeasure
                //      StartCantRight      IfcLengthMeasure
                //      EndCantRight        OPTIONAL IfcLengthMeasure
                //      SmoothingLength     OPTIONAL IfcPositiveLengthMeasure
                //      PredefinedType      IfcAlignmentCantSegmentTypeEnum
                //  END_ENTITY
                //

                //
                //  StartDistAlong
                //
                double  startDistAlong = 0.;
                sdaiGetAttrBN(ifcAlignmentCantSegmentInstance, "StartDistAlong", sdaiREAL, &startDistAlong);

                //
                //  HorizontalLength
                //
                double  horizontalLength = 0.;
                sdaiGetAttrBN(ifcAlignmentCantSegmentInstance, "HorizontalLength", sdaiREAL, &horizontalLength);
                assert(horizontalLength >= 0.);

                //
                //  StartCantLeft
                //
                double  startCantLeft = 0.;
                sdaiGetAttrBN(ifcAlignmentCantSegmentInstance, "StartCantLeft", sdaiREAL, &startCantLeft);

                //
                //  EndCantLeft
                //
                double  endCantLeft = startCantLeft;
                sdaiGetAttrBN(ifcAlignmentCantSegmentInstance, "EndCantLeft", sdaiREAL, &endCantLeft);

                //
                //  StartCantRight
                //
                double  startCantRight = 0.;
                sdaiGetAttrBN(ifcAlignmentCantSegmentInstance, "StartCantRight", sdaiREAL, &startCantRight);

                //
                //  EndCantRight
                //
                double  endCantRight = startCantRight;
                sdaiGetAttrBN(ifcAlignmentCantSegmentInstance, "EndCantRight", sdaiREAL, &endCantRight);

                //
                //  Transition
                //
                if (i == noSegmentInstances - 1) {
                    char    transitionCode[14] = "DISCONTINUOUS";
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "Transition", sdaiENUM, (void*) transitionCode);
                }
                else {
                    char    transitionCode[30] = "CONTSAMEGRADIENTSAMECURVATURE";
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "Transition", sdaiENUM, (void*) transitionCode);
                }

                //
                //  Placement
                //
                ___MATRIX  matrix;
                ___MatrixIdentity(&matrix);

                matrix._41 = startDistAlong - startDistAlongHorizontalAlignment;
                matrix._42 = (startCantLeft + startCantRight) / 2.;
                matrix._43 = 0.;

                if (horizontalLength) {
                    matrix._11 = horizontalLength;
                    matrix._12 = (endCantLeft + endCantRight) / 2. - (startCantLeft + startCantRight) / 2.;
                    matrix._13 = 0.;
                    ___Vec3Normalize((___VECTOR3*) &matrix._11);
                }
                else {
                    matrix._11 = 1.;
                    matrix._12 = 0.;
                    matrix._13 = 0.;
                }

                matrix._21 = -matrix._12;
                matrix._22 = matrix._11;

                double  factor = -(startCantLeft - startCantRight);
                matrix._31 = - factor * matrix._12;
                matrix._32 = factor * matrix._11;
                matrix._33 = railHeadDistance;
                ___Vec3Normalize((___VECTOR3*) &matrix._31);

                ___Vec3Cross((___VECTOR3*) &matrix._21, (___VECTOR3*) &matrix._31, (___VECTOR3*) &matrix._11);

                sdaiPutAttrBN(ifcCurveSegmentInstance, "Placement", sdaiINSTANCE, (void*)___CreateAxis2Placement3D(model, &matrix));

                if ((horizontalLength == 0.) &&
                    (i == noSegmentInstances - 1)) {
                    sdaiPutAttrBN(ifcSegmentedReferenceCurveInstance, "EndPoint", sdaiINSTANCE, (void*) ___CreateAxis2Placement3D(model, &matrix));
                }
                else {
                    assert(horizontalLength > 0.);
                }

                //
                //  Parse the individual segments
                //      BLOSSCURVE
                //      CONSTANTCANT
                //      COSINECURVE
                //      LINEARTRANSITION
                //      HELMERTCURVE
                //      SINECURVE
                //      VIENNESEBEND
                //

                char    * predefinedType = nullptr;
                sdaiGetAttrBN(ifcAlignmentCantSegmentInstance, "PredefinedType", sdaiENUM, &predefinedType);
                if (___equals(predefinedType, (char*) "CONSTANTCANT")) {
                    int_t   ifcLineInstance = ___CreateLine(model);
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcLineInstance);

                    //
                    //  SegmentStart
                    //
                    double  offset = 0.;
                    void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &offset);
                    sdaiPutADBTypePath(segmentStartADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                    double  segmentLength = horizontalLength;

                    //
                    //  SegmentLength
                    //
                    void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                    sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*)segmentLengthADB);
                }
                else if (___equals(predefinedType, (char*) "LINEARTRANSITION")) {
                    ___VECTOR2  orientation = {
                                        horizontalLength,
                                        (endCantLeft + endCantRight) / 2. - (startCantLeft + startCantRight) / 2.
                                    };
                    ___Vec2Normalize(&orientation);
                    int_t   ifcLineInstance = ___CreateLine(model, &orientation);
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcLineInstance);

                    //
                    //  SegmentStart
                    //
                    double  offset = 0.;
                    void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &offset);
                    sdaiPutADBTypePath(segmentStartADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                    double  segmentLength = horizontalLength / orientation.x;

                    //
                    //  SegmentLength
                    //
                    void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                    sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                }
                else if (___equals(predefinedType, (char*) "BLOSSCURVE")) {
                    matrix._11 = 1.;
                    matrix._12 = 0.;
                    matrix._13 = 0.;

                    double  factor__ = -(startCantLeft - startCantRight);
                    matrix._31 = - factor__ * matrix._12;
                    matrix._32 = factor__ * matrix._11;
                    matrix._33 = railHeadDistance;
                    ___Vec3Normalize((___VECTOR3*) &matrix._31);

                    ___Vec3Cross((___VECTOR3*) &matrix._21, (___VECTOR3*) &matrix._31, (___VECTOR3*) &matrix._11);

                    sdaiPutAttrBN(ifcCurveSegmentInstance, "Placement", sdaiINSTANCE, (void*) ___CreateAxis2Placement3D(model, &matrix));

                    double  factor =
                                (endCantLeft + endCantRight) - (startCantLeft + startCantRight) ?
                                    ((endCantLeft + endCantRight) - (startCantLeft + startCantRight)) / (2. * horizontalLength) :
                                    1. / horizontalLength,
                            constantTerm  =   0. * factor + (startCantLeft + startCantRight) / 2.,
                            linearTerm    =   0. * factor,
                            quadraticTerm =   3. * factor,
                            cubicTerm     = - 2. * factor;

                    int_t   ifcBlossCurveInstance =
                                ___CreateBlossCurve(
                                        model,
                                        cubicTerm     ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * cubicTerm),     -1. / 4.) * cubicTerm     / std::fabs(cubicTerm)     : 0.,
                                        quadraticTerm ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * quadraticTerm), -1. / 3.) * quadraticTerm / std::fabs(quadraticTerm) : 0.,
                                        linearTerm    ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * linearTerm),    -1. / 2.) * linearTerm    / std::fabs(linearTerm)    : 0.,
                                        constantTerm  ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * constantTerm),  -1. / 1.) * constantTerm  / std::fabs(constantTerm)  : 0.
                                    );
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcBlossCurveInstance);

                    //
                    //  SegmentStart
                    //
                    double  offsetAsParameter = 0.;
                    void    * segmentStartADB = sdaiCreateADB(sdaiREAL, &offsetAsParameter);
                    sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                    //
                    //  SegmentLength
                    //
                    double  segmentLengthAsLength = horizontalLength;
                    void    * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthAsLength);
                    sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                }
                else if (___equals(predefinedType, (char*) "COSINECURVE")) {
                    matrix._11 = 1.;
                    matrix._12 = 0.;
                    matrix._13 = 0.;

                    double  factor__ = -(startCantLeft - startCantRight);
                    matrix._31 = - factor__ * matrix._12;
                    matrix._32 = factor__ * matrix._11;
                    matrix._33 = railHeadDistance;
                    ___Vec3Normalize((___VECTOR3*) &matrix._31);

                    ___Vec3Cross((___VECTOR3*) &matrix._21, (___VECTOR3*) &matrix._31, (___VECTOR3*) &matrix._11);

                    sdaiPutAttrBN(ifcCurveSegmentInstance, "Placement", sdaiINSTANCE, (void*) ___CreateAxis2Placement3D(model, &matrix));

                    double  factor =
                                (endCantLeft + endCantRight) - (startCantLeft + startCantRight) ?
                                    ((endCantLeft + endCantRight) - (startCantLeft + startCantRight)) / (2. * horizontalLength) :
                                    1. / horizontalLength,
                            constantTerm = 0. * factor + (startCantLeft + startCantRight) / 2.,
                            cosineTerm   = 1. * factor;

                    int_t   ifcSineCurveInstance =
                                ___CreateCosineCurve(
                                        model,
                                        cosineTerm,
                                        constantTerm
                                    );
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcSineCurveInstance);

                    //
                    //  SegmentStart
                    //
                    double  offsetAsParameter = 0.;
                    void    * segmentStartADB = sdaiCreateADB(sdaiREAL, &offsetAsParameter);
                    sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                    //
                    //  SegmentLength
                    //
                    double  segmentLengthAsLength = horizontalLength;
                    void    * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthAsLength);
                    sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                }
                else if (___equals(predefinedType, (char*) "SINECURVE")) {
                    matrix._11 = 1.;
                    matrix._12 = 0.;
                    matrix._13 = 0.;

                    double  factor__ = -(startCantLeft - startCantRight);
                    matrix._31 = - factor__ * matrix._12;
                    matrix._32 = factor__ * matrix._11;
                    matrix._33 = railHeadDistance;
                    ___Vec3Normalize((___VECTOR3*) &matrix._31);

                    ___Vec3Cross((___VECTOR3*) &matrix._21, (___VECTOR3*) &matrix._31, (___VECTOR3*) &matrix._11);

                    sdaiPutAttrBN(ifcCurveSegmentInstance, "Placement", sdaiINSTANCE, (void*) ___CreateAxis2Placement3D(model, &matrix));

                    double  factor =
                                (endCantLeft + endCantRight) - (startCantLeft + startCantRight) ?
                                    ((endCantLeft + endCantRight) - (startCantLeft + startCantRight)) / (2. * horizontalLength) :
                                    1. / horizontalLength,
                            constantTerm = 0. * factor + (startCantLeft + startCantRight) / 2.,
                            sineTerm     = 1. * factor;

                    int_t   ifcSineCurveInstance =
                                ___CreateSineCurve(
                                        model,
                                        sineTerm,
                                        constantTerm,
                                        horizontalLength
                                    );
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcSineCurveInstance);

                    //
                    //  SegmentStart
                    //
                    double  offsetAsParameter = 0.;
                    void    * segmentStartADB = sdaiCreateADB(sdaiREAL, &offsetAsParameter);
                    sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                    //
                    //  SegmentLength
                    //
                    double  segmentLengthAsLength = horizontalLength;
                    void    * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthAsLength);
                    sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                }
                else if (___equals(predefinedType, (char*) "VIENNESEBEND")) {
                    matrix._11 = 1.;
                    matrix._12 = 0.;
                    matrix._13 = 0.;

                    double  factor__ = -(startCantLeft - startCantRight);
                    matrix._31 = - factor__ * matrix._12;
                    matrix._32 = factor__ * matrix._11;
                    matrix._33 = railHeadDistance;
                    ___Vec3Normalize((___VECTOR3*) &matrix._31);

                    ___Vec3Cross((___VECTOR3*) &matrix._21, (___VECTOR3*) &matrix._31, (___VECTOR3*) &matrix._11);

                    sdaiPutAttrBN(ifcCurveSegmentInstance, "Placement", sdaiINSTANCE, (void*) ___CreateAxis2Placement3D(model, &matrix));

                    double  factor =
                                (endCantLeft + endCantRight) - (startCantLeft + startCantRight) ?
                                    ((endCantLeft + endCantRight) - (startCantLeft + startCantRight)) / (2. * horizontalLength) :
                                    1. / horizontalLength,
                            constantTerm  =    0. * factor + (startCantLeft + startCantRight) / 2.,
                            linearTerm    =    0. * factor,
                            quadraticTerm =    0. * factor,
                            cubicTerm     =    0. * factor,
                            quarticTerm   =   35. * factor,
                            quinticTerm   = - 84. * factor,
                            sexticTerm    =   70. * factor,
                            septicTerm    = - 20. * factor;

                    int_t   ifcVienneseBendInstance =
                                ___CreateVienneseBend(
                                        model,
                                        septicTerm    ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * septicTerm),    -1. / 8.) * septicTerm    / std::fabs(septicTerm)    : 0.,
                                        sexticTerm    ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * sexticTerm),    -1. / 7.) * sexticTerm    / std::fabs(sexticTerm)    : 0.,
                                        quinticTerm   ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * quinticTerm),   -1. / 6.) * quinticTerm   / std::fabs(quinticTerm)   : 0.,
                                        quarticTerm   ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * quarticTerm),   -1. / 5.) * quarticTerm   / std::fabs(quarticTerm)   : 0.,
                                        cubicTerm     ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * cubicTerm),     -1. / 4.) * cubicTerm     / std::fabs(cubicTerm)     : 0.,
                                        quadraticTerm ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * quadraticTerm), -1. / 3.) * quadraticTerm / std::fabs(quadraticTerm) : 0.,
                                        linearTerm    ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * linearTerm),    -1. / 2.) * linearTerm    / std::fabs(linearTerm)    : 0.,
                                        constantTerm  ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * constantTerm),  -1. / 1.) * constantTerm  / std::fabs(constantTerm)  : 0.
                                    );
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcVienneseBendInstance);

                    //
                    //  SegmentStart
                    //
                    double  offsetAsParameter = 0.;
                    void    * segmentStartADB = sdaiCreateADB(sdaiREAL, &offsetAsParameter);
                    sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                    //
                    //  SegmentLength
                    //
                    double  segmentLengthAsLength = horizontalLength;
                    void    * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthAsLength);
                    sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                }
                else if (___equals(predefinedType, (char*) "HELMERTCURVE")) {
                    {
                        matrix._11 = 1.;
                        matrix._12 = 0.;
                        matrix._13 = 0.;

                        double  factor__ = -(startCantLeft - startCantRight);
                        matrix._31 = - factor__ * matrix._12;
                        matrix._32 = factor__ * matrix._11;
                        matrix._33 = railHeadDistance;
                        ___Vec3Normalize((___VECTOR3*) &matrix._31);

                        ___Vec3Cross((___VECTOR3*) &matrix._21, (___VECTOR3*) &matrix._31, (___VECTOR3*) &matrix._11);

                        sdaiPutAttrBN(ifcCurveSegmentInstance, "Placement", sdaiINSTANCE, (void*) ___CreateAxis2Placement3D(model, &matrix));

                        double  factor =
                                    ((endCantLeft + endCantRight) - (startCantLeft + startCantRight)) ?
                                        ((endCantLeft + endCantRight) - (startCantLeft + startCantRight)) / (2. * horizontalLength) :
                                        1. / horizontalLength,
                                constantTerm  = 0. * factor + (startCantLeft + startCantRight) / 2.,
                                linearTerm    = 0. * factor,
                                quadraticTerm = 2. * factor;

                        int_t   ifcHelmertCurveInstance =
                                    ___CreateHelmert(
                                            model,
                                            quadraticTerm ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * quadraticTerm), -1. / 3.) * quadraticTerm / std::fabs(quadraticTerm) : 0.,
                                            linearTerm    ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * linearTerm),    -1. / 2.) * linearTerm    / std::fabs(linearTerm)    : 0.,
                                            constantTerm  ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * constantTerm),  -1. / 1.) * constantTerm  / std::fabs(constantTerm)  : 0.,
                                            &matrix
                                        );
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcHelmertCurveInstance);

                        //
                        //  SegmentStart
                        //
                        double  offsetAsParameter = 0.;
                        void    * segmentStartADB = sdaiCreateADB(sdaiREAL, &offsetAsParameter);
                        sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                        //
                        //  SegmentLength
                        //
                        double  segmentLengthAsLength = horizontalLength / 2.;
                        void    * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthAsLength);
                        sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);

                        sdaiAppend((int_t) aggrCurveSegment, sdaiINSTANCE, (void*) ifcCurveSegmentInstance);
                    }

                    matrix._41 += horizontalLength / 2.;
                    matrix._42 += ((endCantLeft + endCantRight) - (startCantLeft + startCantRight)) / 4.;

                    {
                        matrix._11 = 1.;
                        matrix._12 = 0.;
                        matrix._13 = 0.;

                        double  factor__ = -(startCantLeft - startCantRight) + ((endCantLeft + endCantRight) - (startCantLeft + startCantRight)) / 2.;
                        matrix._31 = - factor__ * matrix._12;
                        matrix._32 = factor__ * matrix._11;
                        matrix._33 = railHeadDistance;
                        ___Vec3Normalize((___VECTOR3*) &matrix._31);

                        ___Vec3Cross((___VECTOR3*) &matrix._21, (___VECTOR3*) &matrix._31, (___VECTOR3*) &matrix._11);

                        sdaiPutAttrBN(ifcCurveSegmentInstance, "Placement", sdaiINSTANCE, (void*) ___CreateAxis2Placement3D(model, &matrix));

                        double  factor =
                                    ((endCantLeft + endCantRight) - (startCantLeft + startCantRight)) ?
                                        ((endCantLeft + endCantRight) - (startCantLeft + startCantRight)) / (2. * horizontalLength) :
                                        1. / horizontalLength,
                                constantTerm  = - 1. * factor + (startCantLeft + startCantRight) / 2.,
                                linearTerm    =   4. * factor,
                                quadraticTerm = - 2. * factor;

                        int_t   ifcHelmertCurveInstance =
                                    ___CreateHelmert(
                                            model,
                                            quadraticTerm ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * quadraticTerm), -1. / 3.) * quadraticTerm / std::fabs(quadraticTerm) : 0.,
                                            linearTerm    ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * linearTerm),    -1. / 2.) * linearTerm    / std::fabs(linearTerm)    : 0.,
                                            constantTerm  ? std::fabs(horizontalLength) * pow(std::fabs(horizontalLength * constantTerm),  -1. / 1.) * constantTerm  / std::fabs(constantTerm)  : 0.,
                                            &matrix
                                        );
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcHelmertCurveInstance);

                        //
                        //  SegmentStart
                        //
                        double  offsetAsParameter = 0.;
                        void    * segmentStartADB = sdaiCreateADB(sdaiREAL, &offsetAsParameter);
                        sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                        //
                        //  SegmentLength
                        //
                        double  segmentLengthAsLength = horizontalLength / 2.;
                        void    * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthAsLength);
                        sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                    }
                }
                else {
                    assert(false);
                }

                sdaiAppend((int_t) aggrCurveSegment, sdaiINSTANCE, (void*) ifcCurveSegmentInstance);
            }
        }

        delete[] segmentInstances;
    }

    return  ifcSegmentedReferenceCurveInstance;
}

inline  static  int_t   ___GetAlignmentCant(
                                int_t   model,
                                int_t   ifcAlignmentInstance,
                                bool    * hasIssue
                            )
{
    int_t   ifcAlignmentCantInstance = 0;

    {
	    int_t	* aggrIfcRelAggregates = nullptr, noAggrIfcRelAggregates;
        sdaiGetAttrBN(ifcAlignmentInstance, "IsNestedBy", sdaiAGGR, &aggrIfcRelAggregates);
        noAggrIfcRelAggregates = sdaiGetMemberCount(aggrIfcRelAggregates);
        for (int_t i = 0; i < noAggrIfcRelAggregates; i++) {
            int_t   ifcRelAggregatesInstance = 0;
            engiGetAggrElement(aggrIfcRelAggregates, i, sdaiINSTANCE, &ifcRelAggregatesInstance);

    	    int_t	* aggrIfcObjectDefinition = nullptr, noAggrIfcObjectDefinition;
            sdaiGetAttrBN(ifcRelAggregatesInstance, "RelatedObjects", sdaiAGGR, &aggrIfcObjectDefinition);
            noAggrIfcObjectDefinition = sdaiGetMemberCount(aggrIfcObjectDefinition);
            for (int_t j = 0; j < noAggrIfcObjectDefinition; j++) {
                int_t   ifcObjectDefinitionInstance = 0;
                engiGetAggrElement(aggrIfcObjectDefinition, j, sdaiINSTANCE, &ifcObjectDefinitionInstance);

                if (sdaiGetInstanceType(ifcObjectDefinitionInstance) == sdaiGetEntity(model, "IFCALIGNMENTCANT")) {
                    if (ifcAlignmentCantInstance && hasIssue) {
                        (*hasIssue) = true;
                    }

                    assert(ifcAlignmentCantInstance == 0);
                    ifcAlignmentCantInstance = ifcObjectDefinitionInstance;
                }
            }
        }
    }

    if (ifcAlignmentCantInstance == 0) {
	    int_t	* aggrIfcRelAggregates = nullptr, noAggrIfcRelAggregates;
        sdaiGetAttrBN(ifcAlignmentInstance, "IsDecomposedBy", sdaiAGGR, &aggrIfcRelAggregates);
        noAggrIfcRelAggregates = sdaiGetMemberCount(aggrIfcRelAggregates);
        for (int_t i = 0; i < noAggrIfcRelAggregates; i++) {
            int_t   ifcRelAggregatesInstance = 0;
            engiGetAggrElement(aggrIfcRelAggregates, i, sdaiINSTANCE, &ifcRelAggregatesInstance);

    	    int_t	* aggrIfcObjectDefinition = nullptr, noAggrIfcObjectDefinition;
            sdaiGetAttrBN(ifcRelAggregatesInstance, "RelatedObjects", sdaiAGGR, &aggrIfcObjectDefinition);
            noAggrIfcObjectDefinition = sdaiGetMemberCount(aggrIfcObjectDefinition);
            for (int_t j = 0; j < noAggrIfcObjectDefinition; j++) {
                int_t   ifcObjectDefinitionInstance = 0;
                engiGetAggrElement(aggrIfcObjectDefinition, j, sdaiINSTANCE, &ifcObjectDefinitionInstance);

                if (sdaiGetInstanceType(ifcObjectDefinitionInstance) == sdaiGetEntity(model, "IFCALIGNMENTCANT")) {
                    assert(ifcAlignmentCantInstance == 0);
                    ifcAlignmentCantInstance = ifcObjectDefinitionInstance;

                    if (hasIssue) {
                        (*hasIssue) = true;
                    }
                }
            }
        }
    }

    return  ifcAlignmentCantInstance;
}
