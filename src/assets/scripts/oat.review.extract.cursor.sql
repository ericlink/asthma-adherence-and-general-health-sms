delimiter //
DROP PROCEDURE IF EXISTS oat_review_extract_cursor;
//
CREATE PROCEDURE oat_review_extract_cursor()
BEGIN

DECLARE finished INT DEFAULT 0;
DECLARE userId int;
-- 
DECLARE c1 CURSOR FOR SELECT uid from users;
DECLARE CONTINUE HANDLER FOR NOT FOUND SET finished = 1;
OPEN c1;
my_loop : LOOP
FETCH c1 INTO userId;
IF finished = 1 THEN
	LEAVE my_loop;
END IF;

call oat_review_extract(userId);

END LOOP my_loop;
CLOSE c1;
END;
//
call oat_review_extract_cursor();
//
