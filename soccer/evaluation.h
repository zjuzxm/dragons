// evaluation.h
// 
// Robot independent soccer evaluations.
//
// Created by:  Michael Bowling (mhb@cs.cmu.edu)
//
/* LICENSE:
  =========================================================================
    CMDragons'02 RoboCup F180 Source Code Release
  -------------------------------------------------------------------------
    Copyright (C) 2002 Manuela Veloso, Brett Browning, Mike Bowling,
                       James Bruce; {mmv, brettb, mhb, jbruce}@cs.cmu.edu
    School of Computer Science, Carnegie Mellon University
  -------------------------------------------------------------------------
    This software is distributed under the GNU General Public License,
    version 2.  If you do not have a copy of this licence, visit
    www.gnu.org, or write: Free Software Foundation, 59 Temple Place,
    Suite 330 Boston, MA 02111-1307 USA.  This program is distributed
    in the hope that it will be useful, but WITHOUT ANY WARRANTY,
    including MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  ------------------------------------------------------------------------- */

#ifndef __evaluation_h__
#define __evaluation_h__

#include "world.h"
#include "tactic.h"

class Evaluation {
public:
  // aim()
  //
  // These functions take a target point and two relative vectors
  // denoting the range to aim along.  They take an obs_flags of
  // obstacles to avoid in aiming and then return the the point along
  // the vectors with the largest clear angle.
  //
  // The pref_target_point provides a preferred direction along with a
  // bias.  If no other direction is clear by more than the bias it
  // will simply return the point along the largest open angle near
  // the preference.  Used for hysteresis.
  //
  // aim() should be guaranteed not to return false if obs_flags is 0.
  //

  bool aim(World &world, double time, 
	   vector2d target, vector2d r2, vector2d r1,
	   int obs_flags,
	   vector2d pref_target_point, double pref_amount,
	   vector2d &target_point, double &target_tolerance);

  bool aim(World &world, double time, 
	   vector2d target, vector2d r2, vector2d r1,
	   int obs_flags,
	   vector2d &target_point, double &target_tolerance) {
    return aim(world, time, target, r2, r1, obs_flags, 
	       ((r2 + r1) / 2.0), 0.0,
	       target_point, target_tolerance);
  }

  // defend_line()
  // defend_point()
  // defend_on_line()
  //
  // This returns the position and velocity to use to defend a
  // particular line (or point) on the field.  It combines the
  // positions of the best static defense with the interception point
  // using the variance on the interception point from the Kalman
  // filter.  It also computes the velocity to hit that point with.
  //
  // Setting obs_flags and optionally pref_point and pref_amount can
  // be used to take account for other robots also defending.  When
  // computing a static position it will use aim() with the provided
  // paramters to find the largest remaining open angle and statically
  // defend this range.
  //
  // The defend_*_static() and defend_*_trajectory() methods are
  // helper functions that defend_*() uses.
  //
  // defend_on_line() positions itself along a line segment nearest to
  // the ball.  The intercept flag here still works as biasing the 
  // position towards where the ball will cross the segment.
  //
  // The intercept field specifies whether a moving ball should be
  // intercepted.  If true after the call it means the robot is actively
  // trying to intercept the ball.
  //

  bool defend_line(World &world, double time, 
		   vector2d g1, vector2d g2, 
		   double distmin, double distmax, double dist_off_ball,
		   bool &intercept,
		   int obs_flags, 
		   vector2d pref_point, double pref_amount,
		   vector2d &target, vector2d &velocity);

  bool defend_line(World &world, double time, 
		   vector2d g1, vector2d g2, 
		   double distmin, double distmax, double dist_off_ball,
		   bool &intercept,
		   vector2d &target, vector2d &velocity) {
    return defend_line(world, time, g1, g2, distmin, distmax, dist_off_ball,
		       intercept, 0, vector2d(), 0.0, target, velocity);
  }

  bool defend_point(World &world, double time,
		    vector2d point, 
		    double distmin, double distmax, double dist_off_ball,
		    bool &intercept,
		    vector2d &target, vector2d &velocity);

  bool defend_on_line(World &world, double time,
		      vector2d p1, vector2d p2,
		      bool &intercept,
		      vector2d &target, vector2d &velocity);
  
private:
  bool defend_line_static(World &world, double time,
			  vector2d g1, vector2d g2, double dist,
			  vector2d &target, double &variance);

  bool defend_line_intercept(World &world, double time,
			     vector2d g1, vector2d g2, double dist,
			     vector2d &target, double &variance);

  bool defend_point_static(World &world, double time,
			   vector2d point, double radius,
			   vector2d &target, double &variance);

  bool defend_point_intercept(World &world, double time,
			      vector2d point, double radius,
			      vector2d &target, double &variance);

public:
  vector2d farthest(World &world, double time, 
                    int obs_flags,
                    vector2d bbox_min,vector2d bbox_max,
                    vector2d dir);

  vector2d findOpenPosition(World &world, vector2d p, 
			    vector2d toward, int obs_flags,
			    double pradius = TEAMMATE_EFFECTIVE_RADIUS);

  vector2d findOpenPositionAndYield(World &world, vector2d p,
				    vector2d toward, int obs_flags);
};

extern Evaluation evaluation;

class EvaluationPosition {
public:
  typedef double (*EvalFn)(World &world, const vector2d p, 
			   int obs_flags, double &a);

  TRegion region;

private:
  // Function
  EvalFn eval;

  int obs_flags;

  double last_updated;

  // Evaluated Points
  uint n_points;
  vector<vector2d> points;
  vector<double> angles;
  vector<double> weights;

  vector<vector2d> new_points;

  int best;
  double pref_amount;

  vector2d pointFromDistribution(World &w) {
    return region.sample(w);
  }

public:
  EvaluationPosition() { }
  EvaluationPosition(TRegion _region,
		     EvalFn _eval, 
		     double _pref_amount = 0,
		     int _n_points = 10) {
    set(_region, _eval, _pref_amount, n_points);
  }

  void set(TRegion _region,
	   EvalFn _eval, 
	   double _pref_amount = 0,
	   int _n_points = 10) {
    region = _region;
    eval = _eval; n_points = _n_points;
    pref_amount = _pref_amount;

    obs_flags = 0;

    points.clear();
    weights.clear();
    best = -1;
    last_updated = 0;
  }

  void update(World &world, int _obs_flags) {
    // Only update if world has changed.
    if (world.time <= last_updated) return;
    last_updated = world.time;

    obs_flags = _obs_flags;

    // Check the new points to make sure their within the region.
    // Passed here by addPoint().
    for(uint i = 0; i<new_points.size(); i++)
      if (!region.inRegion(world, new_points[i])) {
	new_points.erase(new_points.begin() + i);
	i--;
      }

    // Add previous best point (or center).
    if (!points.empty()) {
      new_points.push_back(points[best]); best = new_points.size() - 1; 
    } else {
      new_points.push_back(region.center(world)); best = -1;
    }

    // Pick new points.
    while(new_points.size() < n_points)
      new_points.push_back(pointFromDistribution(world));

    points = new_points;
    new_points.clear();
      
    // Evaluate points.
    int best_i = -1;

    weights.clear(); angles.clear();
    for(uint i=0; i<points.size(); i++) {
      double a;

      weights.push_back((eval)(world, points[i], obs_flags, a));
      angles.push_back(a);

      if (best_i < 0 || weights[i] > weights[best_i]) best_i = i;
    }

    if (best < 0 || weights[best_i] > weights[best] + pref_amount) {
      best = best_i;
    }

  }

  void addPoint(vector2d p) {
    for(uint i=0; i<new_points.size(); i++)
      if (new_points[i] == p) return;
    new_points.push_back(p); }

  vector2d point() const {
    return points[best]; }

  double angle() const {
    return angles[best]; }
};

#endif
