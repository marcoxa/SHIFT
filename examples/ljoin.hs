/* -*- Mode: C -*- */

/* ljoin.hs -- */

/*
 * Copyright (c)1996, 1997, 1998 The Regents of the University of
 * California (Regents). All Rights Reserved. 
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research, and not-for-profit purposes,
 * without fee and without a signed licensing agreement, is hereby
 * granted, provided that the above copyright notice, this paragraph and
 * the following two paragraphs appear in all copies, modifications, and
 * distributions. 
 *
 * Contact The Office of Technology Licensing, UC Berkeley, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
 * commercial licensing opportunities. 
 *
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 * SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
 * ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION
 * TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS. 
 */
/*
   This example implements the join maneuver.
   Since it is a stand-alone example the scenario created by type
   "Init" hard-codes several values.
   Three "Supervisor"s (which represent vehicles) are created.
   The frontCar, backCar, leader, and follower varibales are set
   to reflect three cars following each other.
   Sensors are not modeled.
   Communication between JointInitiator and JointResponder is
   established by the Supervisor.
*/
   
   

#define InitLead 0
#define InitMerge 1
#define AccelToMerge 2
#define InitFollow 3

#define SafeToMerge 1 
#define SafeToAmerge 0
#define CompleteMerge 1

global Init myI := create(Init);

type Init {

	output Supervisor v1 := create(Supervisor);
	output Supervisor v2 := create(Supervisor);
	output Supervisor v3 := create(Supervisor);

	discrete a;

	setup
	do {

	frontCar(v2) := v1;
	backCar(v1) := v2;

	frontCar(v3) := v2;
	backCar(v2) := v3;
	
	leader(v1) := v1;
	leader(v2) := v2;
	leader(v3) := v3;

	followers(v1) := {};
	followers(v2) := {};
	followers(v3) := {};

	}
}


type Supervisor
{
	output  Supervisor leader;
		set(Supervisor) followers;
		JoinInitiator jinit := create(JoinInitiator);
		JoinResponder jresp := create(JoinResponder);
		Regulator regulator := create(Regulator);

		Supervisor frontCar := nil;
		Supervisor backCar := nil;
		Supervisor platInMan := nil;
		Supervisor backPlat := nil;

	input	number maxPlatoonSize := 5;

	export	updateJoinInit, updateJoinResp, getLeader, notLeader,
		amLeader, sjoin, joinOkay;

	setup	do {
		supervisor(regulator) := self;
		jinit(regulator) := jinit;

		supervisor(jinit) := self;
		regulator(jinit) := regulator;

		supervisor(jresp) := self;
		}

	discrete
		Leader, WantToJoin, StartJoin, Joining, Follower, 
		FollowingMayJoin, FPJPrep, FollowingPlatoonJoining;

	transition
		Leader -> WantToJoin {getLeader, frontCar:notLeader}
			when frontCar /= nil
			do {
				platInMan := leader(frontCar);
			};

		Leader -> WantToJoin {getLeader, frontCar:amLeader}
			when frontCar /= nil
			do {
				platInMan := frontCar;
			};

		WantToJoin -> StartJoin {sjoin, platInMan:joinOkay}
			do {
				jresp(jinit) := jresp(platInMan);
			};

		StartJoin -> Joining {jinit:joinRequest}
			when size(followers) < maxPlatoonSize - 1;

		Joining -> Leader {jinit:joinAbort}
			do {
				platInMan := nil;
			};

		Joining -> Follower
			{jinit:joinComplete, followers:updateJoinInit(all)}
			do {
				leader := platInMan;
				platInMan := nil;
			//	frontCar := nil;
			};

		Leader -> FollowingMayJoin {backCar:getLeader, amLeader}
			when backCar /= nil
			do {
				platInMan := backCar;
			};

		FollowingMayJoin -> FPJPrep {platInMan:sjoin, joinOkay}
			do {
				jinit(jresp) := jinit(platInMan);
			};

		Leader -> FPJPrep {backPlat:sjoin, joinOkay}
			when backPlat /= nil
			do {
				platInMan := backPlat;
				jinit(jresp) := jinit(backPlat);
			};				

		FPJPrep -> FollowingPlatoonJoining {jresp:joinAck}
			when size(followers) + size(followers(platInMan)) 
					<= maxPlatoonSize -2;

		FollowingPlatoonJoining -> Leader {jresp:joinAbort}
			do {
				platInMan := nil;
				backPlat := nil;
			};

		FollowingPlatoonJoining -> Leader
			{jresp:joinComplete, followers:updateJoinResp(all)}
			do
			{
				followers := followers + {platInMan}
						+ followers(platInMan);
			//	backCar := nil;
				platInMan := nil;
				backPlat := nil;
			};


		Follower -> Follower {updateJoinInit}
			do
			{
				leader := platInMan(leader);
			};

		Follower -> Follower {updateJoinResp}
			do {
				followers := followers + {platInMan(leader)} +
						followers(platInMan(leader));
			};

		Follower -> Follower {backCar:getLeader, notLeader}
			when backCar /= nil
			do {
				backPlat(leader) := backCar;
			};
}



type JoinInitiator
{
	output	JoinResponder jresp;
		Supervisor supervisor;
		Regulator regulator;		

	
	export	joinRequest, joinAbort, joinComplete;

	discrete
		idle, joining;

	transition

		idle -> joining {joinRequest, jresp:joinAck}
			when jresp /= nil;

		joining -> idle {joinAbort, regulator:joinAbort}
			do
			{
				jresp := nil;
			};

		joining -> idle {joinComplete, regulator:joinComplete}
			do
			{
				jresp := nil;
			};
}


type JoinResponder
{
	output	JoinInitiator jinit;
		Supervisor supervisor;


	discrete
		idle, followingPlatoonJoining;

	export  joinAbort, joinAck, joinComplete;

	transition

		idle -> followingPlatoonJoining {jinit:joinRequest, joinAck}
		        when	jinit /= nil;

		followingPlatoonJoining -> idle {jinit:joinAbort, joinAbort}
			do
			{
				jinit := nil;
			};

		followingPlatoonJoining -> idle {jinit:joinComplete,joinComplete}
			do
			{
				jinit := nil;
			};
}


type Regulator
{
	output Supervisor supervisor;
	       JoinInitiator jinit;

	state continuous number t := 0;

	output number retNum;

	export joinAbort, joinComplete;
	
	discrete initial, leader, follower, startjoin , 
		 completejoin {t' = 1};

	transition
		initial -> leader {}
			when supervisor = leader(supervisor)
			do {
				retNum := InitLead;
					};

		leader -> startjoin {jinit:joinRequest}
			do {
				retNum := InitMerge;
				};

		startjoin -> leader {joinAbort}
			when SafeToMerge = 0 ;

		startjoin -> completejoin {}
			when SafeToMerge = 1 
			do {
				retNum := AccelToMerge;
				};

		completejoin -> follower {joinComplete}
			when CompleteMerge = 1 and  t > 0.5
			do {
				retNum := InitFollow;
				};

		completejoin -> leader {joinAbort}
			when SafeToAmerge = 1 and  t > 0.5;

		initial -> follower {}
			when supervisor /= leader(supervisor)
			do {
				retNum := InitFollow;
				};
};

