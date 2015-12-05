delimiter //
DROP PROCEDURE IF EXISTS oat_review_extract;
//
CREATE PROCEDURE oat_review_extract(userId int)
BEGIN

-- other vars
declare 
hfstx_message_log_grand_total,
hfstx_message_log_sent_total,
hfstx_message_log_recd_total,
healthimo_asthma_message_log_grand_total,
healthimo_asthma_message_log_sent_total,
healthimo_asthma_message_log_recd_total,
sent_hfstx_invite,
sent_asthma_invite,
tier1_service_general,
tier2_service_diabetes,
tier2_service_asthma,
min_number_served,
d int;

declare
uname,
ucell,
p1,
p2,
p3,
uaddress,
ucity,
ustate,
uzip,
x varchar(50);

-- 
-- 
--
--
-- hfstx_message_log values
--
--
select count(*) into hfstx_message_log_grand_total from hfstx_message_log where uid = userId;
select count(*) into hfstx_message_log_sent_total from hfstx_message_log where uid = userId and outbound = 1;
select count(*) into hfstx_message_log_recd_total from hfstx_message_log where uid = userId and outbound = 0;

--
--
-- healthimo_asthma_message_log values
--
--
select count(*) into healthimo_asthma_message_log_grand_total from healthimo_asthma_message_log where uid = userId;
select count(*) into healthimo_asthma_message_log_sent_total from healthimo_asthma_message_log where uid = userId and outbound = 1;
select count(*) into healthimo_asthma_message_log_recd_total from healthimo_asthma_message_log where uid = userId and outbound = 0;

-- in asthma import
-- asthma invite sent
-- select count(*) into sent_asthma_invite from import_user_log where log_type = 'invited' and import_table = 'import_user_asthma_dchp_file_01' and uid = userId limit 1;
select count(*) into sent_asthma_invite from import_user_log where log_type = 'invited' and uid = userId limit 1;
-- in rural import
-- health families invite sent
select count(*) into sent_hfstx_invite from import_user_log where log_type = 'welcome' and log_value = 'messaged' and import_table = 'import_user_rural' and uid = userId limit 1;

SELECT count(*) into tier1_service_general FROM users u JOIN profile_values pv ON pv.uid = u.uid WHERE pv.value = 1 AND pv.fid IN ( SELECT pf.fid FROM profile_fields pf WHERE pf.name = 'profile_keyword_healthy') and u.uid = userId;

--
--
-- what is their diabetes status 
--
--
SELECT count(*) into tier2_service_diabetes
FROM users u
JOIN profile_values pv ON pv.uid = u.uid
WHERE pv.value = 1
AND pv.fid IN ( SELECT pf.fid FROM profile_fields pf WHERE pf.name in 
('profile_areas_of_interest_diabetes','profile_has_diabetes_in_household')
)
and u.uid = userId;

if tier2_service_diabetes = 0 then
	select count(*) into tier2_service_diabetes
	from import_user_log l
	where
	rid in (
		select record_id
		from import_user_rural
		where interest_diabetes = 1
	)
	and import_table = 'import_user_rural'
	and l.uid = userId;
end if;

if tier2_service_diabetes > 0 then
	set tier2_service_diabetes = 1;
end if;


--
--
-- what is their asthma status 
--
--
SELECT count(*) into tier2_service_asthma
FROM users u
JOIN profile_values pv ON pv.uid = u.uid
WHERE pv.value = 1
AND pv.fid IN (
	SELECT pf.fid FROM profile_fields pf WHERE pf.name in (
	'profile_keyword_asthma',
	'profile_areas_of_interest_asthma',
	'profile_has_asthma_in_household'
	)
)
and u.uid = userId;

if tier2_service_asthma = 0 then
	select count(*) into tier2_service_asthma
	from import_user_log l
	where rid in (
		select record_id from import_user_rural where interest_asthma = 1
	)
	and import_table = 'import_user_asthma_dchp_file_01'
	and l.uid = userId;
end if;

if tier2_service_asthma > 0 then
	set tier2_service_asthma = 1;
end if;
--
--
-- min # served
--
--
set min_number_served = 1;
if tier2_service_asthma = 1 then
	set min_number_served = 2;
end if;


--
--
-- name
--
--
if uname is null then
	select concat(memberLast, ', ', memberFirst) into uname
	from import_user_log l
	join  import_user_asthma_dchp_file_01 i on l.rid = i.recordId
	where log_type = 'import' 
        and import_table = 'import_user_asthma_dchp_file_01'
	and l.uid = userId
	limit 1;
end if;
if uname is null then
	-- select from import rural
	select concat(l_name, ', ', f_name) into uname
	from import_user_log l
	join  import_user_rural i on l.rid = i.record_id
	where log_type = 'import'
        and import_table = 'import_user_rural'
	and l.uid = userId
	limit 1;
end if;
if uname is null then
	select u.name into uname from users u where u.uid = userId;
end if;


--
--
-- ucell
--
--
if ucell is null then
	select number into ucell from sms_user u where u.uid = userId;
end if;
if ucell is null then
	-- select from import rural
	select cell into ucell
	from import_user_log l
	join  import_user_rural i on l.rid = i.record_id
	where log_type = 'import'
        and import_table = 'import_user_rural'
	and l.uid = userId
	limit 1;
end if;
if ucell is null then
	select phone into ucell
	from import_user_log l
	join  import_user_asthma_dchp_file_01 i on l.rid = i.recordId
	where log_type = 'import'
        and import_table = 'import_user_asthma_dchp_file_01'
	and l.uid = userId
	limit 1;
end if;

--
-- p1
--
--
if p1 is null then
	select phone1 into p1
	from import_user_log l
	join  import_user_asthma_dchp_file_01 i on l.rid = i.recordId
	where log_type = 'import'
        and import_table = 'import_user_asthma_dchp_file_01'
	and l.uid = userId
	limit 1;
end if;

--
-- p2
--
--
if p2 is null then
	select phone2 into p2
	from import_user_log l
	join  import_user_asthma_dchp_file_01 i on l.rid = i.recordId
	where log_type = 'import'
        and import_table = 'import_user_asthma_dchp_file_01'
	and l.uid = userId
	limit 1;
end if;

--
-- p3
--
--
if p3 is null then
	select phone3 into p3
	from import_user_log l
	join  import_user_asthma_dchp_file_01 i on l.rid = i.recordId
	where log_type = 'import'
        and import_table = 'import_user_asthma_dchp_file_01'
	and l.uid = userId
	limit 1;
end if;

--
--
-- uaddress
--
--
if uaddress is null then
	select address into uaddress
	from import_user_log l
	join  import_user_asthma_dchp_file_01 i on l.rid = i.recordId
	where log_type = 'import'
        and import_table = 'import_user_asthma_dchp_file_01'
	and l.uid = userId
	limit 1;
end if;
if uaddress is null then
	-- select from import rural
	select home_address_street into uaddress
	from import_user_log l
	join  import_user_rural i on l.rid = i.record_id
	where log_type = 'import'
        and import_table = 'import_user_rural'
	and l.uid = userId
	limit 1;
end if;



--
--
-- ucity
--
--
if ucity is null then
	select city into ucity
	from import_user_log l
	join  import_user_asthma_dchp_file_01 i on l.rid = i.recordId
	where log_type = 'import'
        and import_table = 'import_user_asthma_dchp_file_01'
	and l.uid = userId
	limit 1;
end if;

--
--
-- state
--
--
if ustate is null then
	select state into ustate
	from import_user_log l
	join  import_user_asthma_dchp_file_01 i on l.rid = i.recordId
	where log_type = 'import'
        and import_table = 'import_user_asthma_dchp_file_01'
	and l.uid = userId
	limit 1;
end if;
if ustate is null then
	-- select from import rural
	select 'TX' into ustate
	from import_user_log l
	join  import_user_rural i on l.rid = i.record_id
	where log_type = 'import'
        and import_table = 'import_user_rural'
	and l.uid = userId
	limit 1;
end if;

--
--
-- uzip
--
--
if uzip is null then
	select zip into uzip
	from import_user_log l
	join  import_user_asthma_dchp_file_01 i on l.rid = i.recordId
	where log_type = 'import'
        and import_table = 'import_user_asthma_dchp_file_01'
	and l.uid = userId
	limit 1;
end if;
if uzip is null then
	select value into uzip
	from profile_values
	where fid in ( select fid from profile_fields where name = 'profile_zip_code') and length(value) = 5
        and uid = userId
	limit 1;
end if;


--    -- ---------------------------------------------------------------------------------
-- message counts
--    select  	userId,
--		hfstx_message_log_grand_total,
--		hfstx_message_log_sent_total,
--		hfstx_message_log_recd_total,
--		healthimo_asthma_message_log_grand_total,
--		healthimo_asthma_message_log_sent_total,
--		healthimo_asthma_message_log_recd_total,
--		sent_hfstx_invite,
--		sent_asthma_invite
--		;

    -- output the extract record
    -- name (if avail), general/asthma/diabetes, ucell, phone 1, p2, p3, p4, p5, uaddress, ucity, state, zip, #messages sent, # messages received, # messages total

    select 
		md5(userId) as 'patient_id', 
		tier1_service_general,
		tier2_service_diabetes,
		tier2_service_asthma,
		min_number_served,
		hfstx_message_log_sent_total + healthimo_asthma_message_log_sent_total + sent_hfstx_invite + sent_asthma_invite as total_messages_sent,
		hfstx_message_log_recd_total + healthimo_asthma_message_log_recd_total as total_messages_received,
		uname as name,
		ucell,
		p1,
		p2,
		p3,
		uaddress,
		ucity,
		ustate as state,
		uzip
		;
END
-- ----------------------------------------------------------
-- ----------------------------------------------------------
-- ----------------------------------------------------------
-- ---END;
//
call oat_review_extract(47552);
//
