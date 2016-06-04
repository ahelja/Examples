#import <Foundation/Foundation.h>


#define ToRad(deg) 		( (M_PI * (deg)) / 180.0 )
#define ToDeg(rad)		( (180.0 * (rad)) / M_PI )
#define SQR(x)			( (x) * (x) )

// Given the vector v from p1 to p2, computes the angle in degrees between v and the compared vector <0,1> (<0,-1> if flipped).
// The returned angle is in the range [0,360).
//
static inline float AngleFromNorth(NSPoint p1, NSPoint p2, BOOL flipped) {
    NSPoint v = NSMakePoint(p2.x-p1.x,p2.y-p1.y);
    float vmag = sqrt(SQR(v.x) + SQR(v.y)), result = 0;
    v.x /= vmag;
    v.y /= vmag;
    result = ToDeg(atan2(v.x,(flipped ? -v.y : v.y)));
    return (result >=0  ? result : result + 360.0);
}
