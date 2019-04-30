
/*
Project: Solver for Rubic cube
Author: David Bolvansky
Login: xbolva00
*/

/* ------------ input2.pl ------------  */
read_line(L,C) :-
	get_char(C),
	(isEOFEOL(C), L = [], !;
		read_line(LL,_),% atom_codes(C,[Cd]),
		[C|LL] = L).

isEOFEOL(C) :-
	C == end_of_file;
	(char_code(C,Code), Code==10).


read_lines(Ls) :-
	read_line(L,C),
	( C == end_of_file, Ls = [] ;
	  read_lines(LLs), Ls = [L|LLs]
	).

split_line([],[[]]) :- !.
split_line([' '|T], [[]|S1]) :- !, split_line(T,S1).
split_line([32|T], [[]|S1]) :- !, split_line(T,S1).    % aby to fungovalo i s retezcem na miste seznamu
split_line([H|T], [[H|G]|S1]) :- split_line(T,[G|S1]). % G je prvni seznam ze seznamu seznamu G|S1

split_lines([],[]).
split_lines([L|Ls],[H|T]) :- split_lines(Ls,T), split_line(L,H).

/* --------------------------------------- */

/* Transform cube from input to better internal form, so transformations are well understandable */
canocalize_cube(
        [
          U_1, U_2, U_3,
          U_4, U_5, U_6,
          U_7, U_8, U_9,
          F_1, F_2, F_3, R_1, R_2, R_3, B_1, B_2, B_3, L_1, L_2, L_3,
          F_4, F_5, F_6, R_4, R_5, R_6, B_4, B_5, B_6, L_4, L_5, L_6,
          F_7, F_8, F_9, R_7, R_8, R_9, B_7, B_8, B_9, L_7, L_8, L_9,
          D_1, D_2, D_3,
          D_4, D_5, D_6,
          D_7, D_8, D_9
        ],
      	[
          [[U_1, U_2, U_3], [U_4, U_5, U_6], [U_7, U_8, U_9]],
          [[F_1, F_2, F_3], [F_4, F_5, F_6], [F_7, F_8, F_9]],
          [[R_1, R_2, R_3], [R_4, R_5, R_6], [R_7, R_8, R_9]],
          [[B_1, B_2, B_3], [B_4, B_5, B_6], [B_7, B_8, B_9]],
          [[L_1, L_2, L_3], [L_4, L_5, L_6], [L_7, L_8, L_9]],
          [[D_1, D_2, D_3], [D_4, D_5, D_6], [D_7, D_8, D_9]]
        ]).

/* How solution looks like */
is_solved_cube([
             [[U,U,U],[U,U,U],[U,U,U]],
             [[F,F,F],[F,F,F],[F,F,F]],
             [[R,R,R],[R,R,R],[R,R,R]],
             [[B,B,B],[B,B,B],[B,B,B]],
             [[L,L,L],[L,L,L],[L,L,L]],
             [[D,D,D],[D,D,D],[D,D,D]]
            ]).

/* Try possible rotations */
rotate_cube(Cube, [], Cube).
rotate_cube(Cube, [Rotation|Rest], NewCube) :- rotate_cube(RotatedCube, Rest, NewCube), transform_cube(Rotation, Cube, RotatedCube).

/* Solve cube by applying transformations (rotations) */
solve_cube(Cube, Solution) :- rotate_cube(Cube, Solution, RotatedCube), is_solved_cube(RotatedCube).

/* Print cube in the required format */
print_cube([
            [[U_1, U_2, U_3], [U_4, U_5, U_6], [U_7, U_8, U_9]],
            [[F_1, F_2, F_3], [F_4, F_5, F_6], [F_7, F_8, F_9]],
            [[R_1, R_2, R_3], [R_4, R_5, R_6], [R_7, R_8, R_9]],
            [[B_1, B_2, B_3], [B_4, B_5, B_6], [B_7, B_8, B_9]],
            [[L_1, L_2, L_3], [L_4, L_5, L_6], [L_7, L_8, L_9]],
            [[D_1, D_2, D_3], [D_4, D_5, D_6], [D_7, D_8, D_9]]
           ]) :-
        format("~w~w~w\n~w~w~w\n~w~w~w\n", [U_1, U_2, U_3, U_4, U_5, U_6, U_7, U_8, U_9]),
        format("~w~w~w ~w~w~w ~w~w~w ~w~w~w\n", [F_1, F_2, F_3, R_1, R_2, R_3, B_1, B_2, B_3, L_1, L_2, L_3]),
        format("~w~w~w ~w~w~w ~w~w~w ~w~w~w\n", [F_4, F_5, F_6, R_4, R_5, R_6, B_4, B_5, B_6, L_4, L_5, L_6]),
        format("~w~w~w ~w~w~w ~w~w~w ~w~w~w\n", [F_7, F_8, F_9, R_7, R_8, R_9, B_7, B_8, B_9, L_7, L_8, L_9]),
        format("~w~w~w\n~w~w~w\n~w~w~w", [D_1, D_2, D_3, D_4, D_5, D_6, D_7, D_8, D_9]).

/* Print solution */
print_solution(_, []).
print_solution(Cube, [Rotation|Rest]) :- transform_cube(Rotation, Cube, RotatedCube), nl, nl, print_cube(RotatedCube), print_solution(RotatedCube, Rest).

/* Rotate up side clockwise */
transform_cube(rotate_up_clockwise,
        [
          [[U_1, U_2, U_3], [U_4, U_5, U_6], [U_7, U_8, U_9]],
          [F123, F456, F789],
          [R123, R456, R789],
          [B123, B456, B789],
          [L123, L456, L789],
          D123456789
        ],
        [
          [[U_7, U_4, U_1], [U_8, U_5, U_2], [U_9, U_6, U_3]],
          [R123, F456, F789],
          [B123, R456, R789],
          [L123, B456, B789],
          [F123, L456, L789],
          D123456789
        ]).

/* Rotate up side counterclockwise */
transform_cube(rotate_up_counterclockwise, C1, C2) :- transform_cube(rotate_up_clockwise, C2, C1).

/* Rotate down side clockwise */
transform_cube(rotate_down_clockwise,
        [
          U123456789,
          [F123, F456, F789],
          [R123, R456, R789],
          [B123, B456, B789],
          [L123, L456, L789],
          [[D_1, D_2, D_3], [D_4, D_5, D_6], [D_7, D_8, D_9]]
        ],
        [
          U123456789,
          [F123, F456, L789],
          [R123, R456, F789],
          [B123, B456, R789],
          [L123, L456, B789],
          [[D_7, D_4, D_1], [D_8, D_5, D_2], [D_9, D_6, D_3]]
        ]).


/* Rotate down side counterclockwise */
transform_cube(rotate_down_counterclockwise, C1, C2) :- transform_cube(rotate_down_clockwise, C2, C1).

/* Rotate front side clockwise */
transform_cube(rotate_front_clockwise,
        [
          [U123, U456, [U_7, U_8, U_9]],
          [[F_1, F_2, F_3], [F_4, F_5, F_6], [F_7, F_8, F_9]],
          [[R_1|R23], [R_4|R56], [R_7|R89]],
          B123456789,
          [[L_1, L_2, L_3], [L_4, L_5, L_6], [L_7, L_8, L_9]],
          [[D_1, D_2, D_3], D456, D789]
        ],
        [
          [U123, U456, [L_3, L_6, L_9]],
          [[F_7, F_4, F_1], [F_8, F_5, F_2], [F_9, F_6, F_3]],
          [[U_7|R23], [U_8|R56], [U_9|R89]],
          B123456789,
          [[L_1, L_2, D_1], [L_4, L_5, D_2], [L_7, L_8, D_3]],
          [[R_7, R_4, R_1], D456, D789]
        ]).

/* Rotate front side anti clockwise */
transform_cube(rotate_front_counterclockwise, C1, C2) :- transform_cube(rotate_front_clockwise, C2, C1).

/* Rotate back side clockwise */
transform_cube(rotate_back_clockwise,
        [
          [[U_1, U_2, U_3], U456, U789],
          F123456789,
          [[R_1, R_2, R_3], [R_4, R_5, R_6], [R_7, R_8, R_9]],
          [[B_1, B_2, B_3], [B_4, B_5, B_6], [B_7, B_8, B_9]],
          [[L_1|L23], [L_4|L56], [L_7|L89]],
          [D123, D456, [D_7, D_8, D_9]]
        ],
        [
          [[R_3, R_6, R_9], U456, U789],
          F123456789,
          [[R_1, R_2, D_9], [R_4, R_5, D_8], [R_7, R_8, D_7]],
          [[B_7, B_4, B_1], [B_8, B_5, B_2], [B_9, B_6, B_3]],
          [[U_1|L23], [U_2|L56], [U_3|L89]],
          [D123, D456, [L_1, L_4, L_7]]
        ]).

/* Rotate back side counterclockwise */
transform_cube(rotate_back_counterclockwise, C1, C2) :- transform_cube(rotate_back_clockwise, C2, C1).

/* Rotate left side clockwise */
transform_cube(rotate_left_clockwise,
        [
          [[U_1|U23], [U_4|U56], [U_7|U89]],
          [[F_1|F23], [F_4|F56], [F_7|F89]],
          R123456789,
          [[B_1, B_2, B_3], [B_4, B_5, B_6], [B_7, B_8, B_9]],
          [[L_1, L_2, L_3], [L_4, L_5, L_6], [L_7, L_8, L_9]],
          [[D_1|D23], [D_4|D56], [D_7|D89]]
        ],
        [
          [[B_9|U23], [B_6|U56], [B_3|U89]],
          [[U_1|F23], [U_4|F56], [U_7|F89]],
          R123456789,
          [[B_1, B_2, D_7], [B_4, B_5, D_4], [B_7, B_8, D_1]],
          [[L_7, L_4, L_1], [L_8, L_5, L_2], [L_9, L_6, L_3]],
          [[F_1|D23], [F_4|D56], [F_7|D89]]
        ]).

/* Rotate left side counterclockwise */
transform_cube(rotate_left_counterclockwise, C1, C2) :- transform_cube(rotate_left_clockwise, C2, C1).

/* Rotate right side clockwise */
transform_cube(rotate_right_clockwise,
        [
          [[U_1, U_2, U_3], [U_4, U_5, U_6], [U_7, U_8, U_9]],
          [[F_1, F_2, F_3], [F_4, F_5, F_6], [F_7, F_8, F_9]],
          [[R_1, R_2, R_3], [R_4, R_5, R_6], [R_7, R_8, R_9]],
          [[B_1, B_2, B_3], [B_4, B_5, B_6], [B_7, B_8, B_9]],
          L123456789,
          [[D_1, D_2, D_3], [D_4, D_5, D_6], [D_7, D_8, D_9]]
        ],
        [
          [[U_1, U_2, F_3], [U_4, U_5, F_6], [U_7, U_8, F_9]],
          [[F_1, F_2, D_3], [F_4, F_5, D_6], [F_7, F_8, D_9]],
          [[R_7, R_4, R_1], [R_8, R_5, R_2], [R_9, R_6, R_3]],
          [[U_9, B_2, B_3], [U_6, B_5, B_6], [U_3, B_8, B_9]],
          L123456789,
          [[D_1, D_2, B_7], [D_4, D_5, B_4], [D_7, D_8, B_1]]
        ]).

/* Rotate right side counterclockwise */
transform_cube(rotate_right_counterclockwise, C1, C2) :- transform_cube(rotate_right_clockwise, C2, C1).

main :-
        prompt(_, ''),
        read_lines(Lines),
        split_lines(Lines, SplitedLines),
        flatten(SplitedLines, RawCube),
        canocalize_cube(RawCube, Cube),
        print_cube(Cube),
        solve_cube(Cube, Solution),
        print_solution(Cube, Solution),
        halt.
