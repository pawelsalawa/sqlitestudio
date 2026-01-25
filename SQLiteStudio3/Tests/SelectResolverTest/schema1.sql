CREATE TABLE billing_events (
    id TEXT (36) PRIMARY KEY NOT NULL,
    occurred_at INTEGER NOT NULL,
    domain TEXT NOT NULL,
    label TEXT NOT NULL,
    details TEXT,
    amount_scaled INTEGER NOT NULL,
    currency_code TEXT (3) NOT NULL,
    metadata_json TEXT,
    related_record_url TEXT REFERENCES records (url),
    duration_ms INTEGER NOT NULL
);

CREATE TABLE records (
    url TEXT NOT NULL PRIMARY KEY,
    headline TEXT,
    abstract TEXT,
    credited_to TEXT,
    published_ts INTEGER,
    body_raw TEXT NOT NULL,
    ingested_ts INTEGER,
    source_url TEXT,
    error_count INTEGER NOT NULL DEFAULT 0,
    last_error TEXT,
    enabled INTEGER NOT NULL DEFAULT 1,
    FOREIGN KEY (source_url) REFERENCES sources (url)
);

CREATE TABLE sources (
    url TEXT NOT NULL PRIMARY KEY UNIQUE,
    enabled BOOLEAN NOT NULL DEFAULT 1,
    cohort TEXT
);

CREATE TABLE record_annotations (
    id TEXT NOT NULL PRIMARY KEY,
    record_url TEXT NOT NULL REFERENCES records (url),
    revision INTEGER NOT NULL,
    invalid_input BOOLEAN NOT NULL DEFAULT 0,
    inferred_author TEXT,
    inferred_date INTEGER,
    category_main TEXT,
    category_sub TEXT,
    tags_csv TEXT,
    quality_score REAL,
    accuracy_score REAL,
    complexity_score REAL,
    longevity_score REAL,
    engagement_score REAL,
    content_kind TEXT,
    suggested_participant TEXT,
    UNIQUE (record_url, revision)
);

CREATE TABLE record_heuristics (
    id TEXT PRIMARY KEY,
    simplified_headline TEXT,
    record_url TEXT REFERENCES records (url),
    revision INTEGER NOT NULL,
    score REAL NOT NULL,
    rationale TEXT,
    region TEXT,
    subregion TEXT,
    domain TEXT,
    subtype TEXT,
    UNIQUE (record_url, revision)
);

CREATE TABLE narratives (
    id TEXT NOT NULL PRIMARY KEY,
    record_url TEXT NOT NULL,
    draft_text TEXT,
    final_text TEXT NOT NULL,
    language TEXT,
    aux_payload_json TEXT,
    version TEXT NOT NULL,
    active INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE renderings (
    id TEXT PRIMARY KEY NOT NULL,
    narrative_id TEXT NOT NULL REFERENCES narratives (id),
    engine TEXT NOT NULL,
    engine_version TEXT NOT NULL,
    parameters_json TEXT NOT NULL DEFAULT '{}',
    markup TEXT NOT NULL,
    channel TEXT NOT NULL,
    output_path TEXT NOT NULL,
    captions TEXT NOT NULL,
    active INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE releases (
    id TEXT PRIMARY KEY NOT NULL,
    rendering_id TEXT NOT NULL REFERENCES renderings (id),
    description TEXT NOT NULL DEFAULT '',
    title TEXT NOT NULL,
    uploaded_ts INTEGER,
    scheduled_ts INTEGER,
    media_path TEXT NOT NULL,
    thumbnail_path TEXT NOT NULL,
    channel_key TEXT NOT NULL,
    tags TEXT NOT NULL DEFAULT '',
    active INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE release_sources (
    release_id TEXT NOT NULL,
    record_url TEXT NOT NULL,
    PRIMARY KEY (release_id, record_url),
    FOREIGN KEY (release_id) REFERENCES releases (id)
);

CREATE TABLE records_flagged_as_news (
    url PRIMARY KEY REFERENCES records (url) NOT NULL
);

CREATE INDEX idx_records_time ON records (published_ts DESC, ingested_ts DESC);
CREATE INDEX idx_annotations_date ON record_annotations (inferred_date DESC);
CREATE INDEX idx_annotations_scores ON record_annotations (
    engagement_score DESC,
    quality_score DESC,
    longevity_score DESC
);
CREATE INDEX idx_heuristics_score ON record_heuristics (score DESC);
CREATE INDEX idx_narratives_record_lang_ver ON narratives (record_url, language, version);

-- =========================
-- Views
-- =========================

CREATE VIEW v_all_records AS
SELECT
    r.url,
    r.headline,
    h.simplified_headline,
    r.abstract,
    r.credited_to,
    r.published_ts,
    r.body_raw,
    r.ingested_ts,
    r.source_url,
    s.cohort,
    r.error_count,
    r.last_error,
    r.enabled,
    a.invalid_input,
    a.inferred_author,
    a.inferred_date,
    a.suggested_participant,
    a.category_main,
    a.category_sub,
    a.tags_csv,
    a.quality_score,
    a.accuracy_score,
    a.complexity_score,
    a.longevity_score,
    a.engagement_score,
    a.content_kind,
    h.region,
    h.subregion,
    h.score,
    h.domain,
    h.subtype
FROM records r
LEFT JOIN record_annotations a ON r.url = a.record_url
LEFT JOIN record_heuristics h ON r.url = h.record_url
LEFT JOIN sources s ON r.source_url = s.url
ORDER BY COALESCE(r.published_ts, r.ingested_ts, a.inferred_date) DESC;

CREATE VIEW v_valid_records AS
SELECT *
FROM v_all_records
WHERE
    error_count = 0
    AND enabled = 1
    AND (content_kind <> 'empty' OR subtype <> 'empty')
    AND (invalid_input = 0 OR invalid_input IS NULL);

CREATE VIEW v_pending_narratives AS
SELECT *
FROM v_valid_records vr
WHERE EXISTS (
    SELECT 1
    FROM narratives n
    WHERE n.record_url = vr.url
      AND n.active = 1
      AND NOT EXISTS (
          SELECT 1
          FROM renderings r
          WHERE r.narrative_id = n.id
      )
);

CREATE VIEW v_pending_releases AS
SELECT *
FROM v_valid_records vr
JOIN narratives n ON vr.url = n.record_url AND n.active = 1
JOIN renderings r ON n.id = r.narrative_id AND r.active = 1
WHERE r.id NOT IN (
    SELECT rendering_id FROM releases
);

CREATE VIEW v_new_records AS
SELECT *
FROM v_valid_records vr
WHERE NOT EXISTS (
    SELECT 1 FROM narratives n WHERE n.record_url = vr.url
)
AND NOT EXISTS (
    SELECT 1 FROM records_flagged_as_news f WHERE f.url = vr.url
)
AND NOT EXISTS (
    SELECT 1 FROM release_sources rs WHERE rs.record_url = vr.url
);

CREATE VIEW v_unscored_records AS
SELECT *
FROM v_all_records
WHERE score IS NULL;

CREATE VIEW v_unannotated_records AS
SELECT *
FROM v_valid_records
WHERE longevity_score IS NULL AND score IS NOT NULL;

CREATE VIEW v_time_sensitive_records AS
SELECT *
FROM v_valid_records vr
WHERE
    NOT EXISTS (
        SELECT 1 FROM records_flagged_as_news f WHERE f.url = vr.url
    )
    AND (
        longevity_score <= 0.2
        OR domain = 'news'
        OR subtype = 'news'
    )
    AND (
        inferred_date >= strftime('%s', 'now', '-7 days')
        OR published_ts >= strftime('%s', 'now', '-7 days')
        OR ingested_ts >= strftime('%s', 'now', '-7 days')
    )
ORDER BY
    published_ts DESC,
    inferred_date DESC,
    ingested_ts DESC,
    score DESC,
    engagement_score DESC,
    quality_score DESC,
    longevity_score DESC;

CREATE VIEW v_all_releases AS
SELECT
    rel.id AS release_id,
    ren.id AS rendering_id,
    nar.id AS narrative_id,
    rec.*,
    rel.description AS release_description,
    rel.title AS release_title,
    rel.uploaded_ts,
    rel.scheduled_ts,
    rel.media_path,
    rel.thumbnail_path,
    rel.channel_key,
    rel.tags,
    ren.captions
FROM releases rel
JOIN renderings ren ON rel.rendering_id = ren.id
JOIN narratives nar ON ren.narrative_id = nar.id
JOIN v_all_records rec ON nar.record_url = rec.url
WHERE rel.active = 1
ORDER BY rel.id DESC;

CREATE VIEW v_unsent_releases AS
SELECT *
FROM v_all_releases
WHERE uploaded_ts IS NULL AND scheduled_ts IS NULL;

CREATE VIEW v_sent_releases AS
SELECT *
FROM v_all_releases
WHERE uploaded_ts IS NOT NULL OR scheduled_ts IS NOT NULL;

INSERT INTO sources (url, enabled, cohort) VALUES
('source://alpha', 1, 'primary'),
('source://beta', 1, 'secondary');

INSERT INTO records (
    url, headline, abstract, credited_to,
    published_ts, body_raw, ingested_ts,
    source_url, error_count, enabled
) VALUES
(
    'record://001',
    'Synthetic cognition trends',
    'Overview of current cognitive architectures',
    'A. Researcher',
    strftime('%s','now','-2 days'),
    'Full body text A',
    strftime('%s','now','-2 days'),
    'source://alpha',
    0,
    1
),
(
    'record://002',
    'Distributed reasoning systems',
    'Analysis of multi-agent inference',
    'B. Analyst',
    strftime('%s','now','-10 days'),
    'Full body text B',
    strftime('%s','now','-9 days'),
    'source://alpha',
    0,
    1
),
(
    'record://003',
    'Obsolete draft',
    'Broken input example',
    'C. Author',
    strftime('%s','now','-1 days'),
    'Broken body',
    strftime('%s','now','-1 days'),
    'source://beta',
    1,
    1
);

INSERT INTO record_annotations (
    id, record_url, revision, invalid_input,
    inferred_author, inferred_date,
    category_main, category_sub, tags_csv,
    quality_score, accuracy_score,
    complexity_score, longevity_score,
    engagement_score, content_kind,
    suggested_participant
) VALUES
(
    'ann-001', 'record://001', 1, 0,
    'A. Researcher',
    strftime('%s','now','-2 days'),
    'technology', 'ai',
    'ai,cognition,systems',
    0.9, 0.85, 0.8, 0.6, 0.7,
    'analysis',
    'Expert Guest'
),
(
    'ann-002', 'record://002', 1, 0,
    'B. Analyst',
    strftime('%s','now','-10 days'),
    'science', 'distributed',
    'agents,inference',
    0.7, 0.75, 0.7, 0.9, 0.5,
    'essay',
    NULL
);

INSERT INTO record_heuristics (
    id, simplified_headline, record_url,
    revision, score, rationale,
    region, subregion, domain, subtype
) VALUES
(
    'heu-001',
    'Cognition trends',
    'record://001',
    1,
    0.88,
    'High engagement potential',
    'global', 'eu',
    'analysis', 'news'
),
(
    'heu-002',
    'Distributed reasoning',
    'record://002',
    1,
    0.55,
    'Evergreen technical content',
    'global', 'us',
    'science', 'longform'
);

INSERT INTO narratives (
    id, record_url, draft_text,
    final_text, language, version, active
) VALUES
(
    'nar-001',
    'record://001',
    'Draft script A',
    'Final narrative A',
    'en',
    'v1',
    1
),
(
    'nar-002',
    'record://002',
    'Draft script B',
    'Final narrative B',
    'en',
    'v1',
    1
);

INSERT INTO renderings (
    id, narrative_id,
    engine, engine_version,
    markup, channel,
    output_path, captions, active
) VALUES
(
    'ren-001',
    'nar-001',
    'neural-tts',
    '1.0',
    '<ssml>Audio A</ssml>',
    'audio',
    '/audio/a.mp3',
    'Captions A',
    1
);

INSERT INTO releases (
    id, rendering_id,
    description, title,
    uploaded_ts, scheduled_ts,
    media_path, thumbnail_path,
    channel_key, tags, active
) VALUES
(
    'rel-001',
    'ren-001',
    'Release of cognitive trends',
    'Cognition Trends Episode',
    NULL,
    NULL,
    '/media/a.mp3',
    '/thumbs/a.png',
    'main',
    'ai,technology',
    1
);

INSERT INTO release_sources (release_id, record_url) VALUES
('rel-001', 'record://001');

INSERT INTO records_flagged_as_news (url) VALUES
('record://001');

