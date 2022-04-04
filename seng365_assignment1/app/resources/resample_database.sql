INSERT INTO Category (category_id, name)
VALUES (1, 'Animals'),
       (2, 'Environment'),
       (3, 'Entertainment'),
       (4, 'Human rights'),
       (5, 'Immigration'),
       (6, 'Justice'),
       (7, 'Other');

INSERT INTO Petition (petition_id, title, description, author_id, category_id, created_date, closing_date,
                      photo_filename)
VALUES (1, 'Don\'t take The Edge TV off the air', 'MediaWorks have said that "From 01 July 2019, ThreeLife +1 will broadcast on Freeview channel 14 while Edge TV transitions to a HD streaming proposition on ThreeNow, rova and theedge.co.nz." (see http://www.screenscribe.net/channels/hot-off-the-press-release-edge-tv-to-go-hd/ for the full press release).

This means that there will be no more music television channels on free-to-air TV in NZ, ending 16 years of free-to-air music TV that started with C4 in 2003. Music TV is great to put on in the background, especially at businesses such as fast food restaurants, airports, doctors waiting rooms, etc.

Adding a +1 channel for ThreeLife will enable people who miss a TV show by an hour to watch it - but they can already do this using ThreeNow.

While it will still be possible to put The Edge TV on a TV, such as plugging in a computer or over Chromecast, this requires another device (as well as a TV), and a good internet connection. This will probably be too much effort for most businesses, and a large portion of home users.

The content for The Edge TV is still being created for online, why not keep broadcasting it over terrestrial?', 1, 3,
        '2020-01-23', '2020-12-31', 'petition_1.jpg'),
       (2, 'Make CANTA Editorially Independent from the UCSA',
        'Canta should not be censored by the UCSA.', 2, 7, '2020-01-12',
        '2020-12-12', 'petition_2.png'),
       (3, 'Ban all single-use plastic', 'It\'s not good for the environment', 3, 2, '2020-01-19', null,
        'petition_3.jpg'),
       (4, 'Increase the refugee quota', 'New Zealand currently accepts 1000 refugees. It should be more.', 4, 5,
        '2019-10-30', '2020-01-01', 'petition_4.jpg'),
       (5, 'Introduce koalas to New Zealand',
        'Koalas are functionally extinct in Australia, and could thrive in New Zealand, as many other Australasian species do.  They would not compromise our local eco-system, as koalas typically inhabit open eucalypt woodlands, and the leaves of these trees make up most of their diet.',
        5, 1, '2020-01-02', '2020-12-02', 'petition_5.jpg'),
       (6, 'Declare a climate emergency in Australia',
        'Australia\'s increasing deadly wildfires are caused by climate change. We need to do something about it.', 6,
        2, '2020-01-04', '2021-01-04', 'petition_6.jpg'),
       (7, 'Make school uniforms optional',
        'They should be compulsory when students are representing the school offsite, but not every day.', 7, 4,
        '2020-01-11', '2020-11-11', 'petition_7.jpg'),
       (8, 'Make the minimum wage the living wage',
        'The current minimum wage is not enough to live on. It should be increased to $21.15.', 8, 4, '2020-02-04',
        null, 'petition_8.jpg'),
       (9, 'Bring back $2 rice!', '$2.50 rice just isn\'t the same', 9, 7, '2024-05-01', null, 'petition_9.jpg'),
       (10, 'Erect a statue for the Souvlaki man', 'His work deserves to be honoured.', 10, 7, '2020-01-01',
        '2020-12-31', 'petition_10.jpg'),
       (11, 'Make the My Timetable timeout duration longer', 'My Timetable times out after I go onto another tab for a few minutes, forcing me to log back in again to use it.

There should be the option to disable, or least increase, the timout.', 1, 7, '2020-01-19', null, 'petition_11.png'),
       (12, 'Reduce UC parking costs', 'It costs too much to park on campus', 4, 7, '2020-01-13', '2020-12-05',
        'petition_12.jpg');

INSERT INTO Signature (signatory_id, petition_id, signed_date)
VALUES
    # First, everyone signs their own petition the same time it was created.
    (1, 1, '2020-01-23'),
    (2, 2, '2020-01-12'),
    (3, 3, '2020-01-19'),
    (4, 4, '2019-10-30'),
    (5, 5, '2020-01-02'),
    (6, 6, '2020-01-04'),
    (7, 7, '2020-01-11'),
    (8, 8, '2020-02-04'),
    (9, 9, '2020-02-01'),
    (10, 10, '2020-01-01'),
    (1, 11, '2020-01-19'),
    (4, 12, '2020-01-13'),
    # Now people sign some other petitions after they were created.
    (10, 1, '2020-01-24'),
    (2, 1, '2020-02-01'),
    (3, 1, '2020-02-02'),
    (5, 1, '2020-02-04'),
    (9, 1, '2020-02-05'),
    (1, 2, '2020-01-13'),
    (3, 4, '2019-12-05'),
    (8, 4, '2019-12-05'),
    (3, 5, '2020-02-05'),
    (3, 6, '2020-02-05'),
    (3, 7, '2020-02-05'),
    (4, 7, '2020-01-15'),
    (3, 8, '2020-02-05'),
    (3, 9, '2020-02-05'),
    (3, 10, '2020-02-05'),
    (3, 11, '2020-02-05'),
    (3, 12, '2020-02-05'),
    (5, 12, '2020-01-19'),
    (1, 12, '2020-01-19');
