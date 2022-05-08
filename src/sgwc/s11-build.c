/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "s11-build.h"

ogs_pkbuf_t *sgwc_s11_build_create_session_response(
        uint8_t type, sgwc_sess_t *sess)
{
    int rv;
#if 0
    sgwc_bearer_t *bearer = NULL;
#endif
    sgwc_ue_t *sgwc_ue = NULL;

    ogs_gtp2_message_t gtp_message;
    ogs_gtp2_create_session_response_t *rsp = NULL;

    ogs_gtp2_cause_t cause;

    ogs_gtp2_f_teid_t sgw_s11_teid;
    int len;
#if 0
    int i;
    ogs_gtp2_cause_t bearer_cause[OGS_BEARER_PER_UE];
    ogs_gtp2_f_teid_t pgw_s5u_teid[OGS_BEARER_PER_UE];
    int pgw_s5u_len[OGS_BEARER_PER_UE];
#endif

    ogs_debug("[SGWC] Create Session Response");

    ogs_assert(sess);
    sgwc_ue = sess->sgwc_ue;
    ogs_assert(sgwc_ue);

#if 0
    ogs_debug("    SGW_S5C_TEID[0x%x] SMF_N4_TEID[0x%x]",
            sess->sgw_s5c_teid, sess->sgwc_n4_teid);
#endif

    rsp = &gtp_message.create_session_response;
    memset(&gtp_message, 0, sizeof(ogs_gtp2_message_t));

    /* Set Cause */
    memset(&cause, 0, sizeof(cause));
    rsp->cause.presence = 1;
    rsp->cause.len = sizeof(cause);
    rsp->cause.data = &cause;

    cause.value = OGS_GTP2_CAUSE_REQUEST_ACCEPTED;

    /* Send Control Plane(UL) : SGW-S11 */
    memset(&sgw_s11_teid, 0, sizeof(ogs_gtp2_f_teid_t));
    sgw_s11_teid.interface_type = OGS_GTP2_F_TEID_S11_S4_SGW_GTP_C;
    sgw_s11_teid.teid = htobe32(sgwc_ue->sgw_s11_teid);
    rv = ogs_gtp2_sockaddr_to_f_teid(
            ogs_gtp_self()->gtpc_addr, ogs_gtp_self()->gtpc_addr6,
            &sgw_s11_teid, &len);
    ogs_assert(rv == OGS_OK);
    rsp->sender_f_teid_for_control_plane.presence = 1;
    rsp->sender_f_teid_for_control_plane.data = &sgw_s11_teid;
    rsp->sender_f_teid_for_control_plane.len = len;

#if 0
    i = 0;
    ogs_list_for_each(&sess->bearer_list, bearer) {
        ogs_assert(i < OGS_BEARER_PER_UE);

        ogs_debug("    SGW_S5U_TEID[0x%x] PGW_S5U_TEID[0x%x]",
                bearer->sgw_s5u_teid, bearer->pgw_s5u_teid);

        /* Bearer EBI */
        rsp->bearer_contexts_created[i].presence = 1;
        rsp->bearer_contexts_created[i].eps_bearer_id.presence = 1;
        rsp->bearer_contexts_created[i].eps_bearer_id.u8 = bearer->ebi;

        /* Bearer Cause */
        memset(&bearer_cause[i], 0, sizeof(bearer_cause[i]));
        rsp->bearer_contexts_created[i].cause.presence = 1;
        rsp->bearer_contexts_created[i].cause.len = sizeof(bearer_cause[i]);
        rsp->bearer_contexts_created[i].cause.data = &bearer_cause[i];
        bearer_cause[i].value = OGS_GTP2_CAUSE_REQUEST_ACCEPTED;

        /* Bearer QoS
         * if PCRF changes Bearer QoS, this should be included. */
        if (sess->gtp.create_session_response_bearer_qos == true) {
            memset(&bearer_qos, 0, sizeof(bearer_qos));
            bearer_qos.qci = sess->session.qos.index;
            bearer_qos.priority_level = sess->session.qos.arp.priority_level;
            bearer_qos.pre_emption_capability =
                sess->session.qos.arp.pre_emption_capability;
            bearer_qos.pre_emption_vulnerability =
                sess->session.qos.arp.pre_emption_vulnerability;

            rsp->bearer_contexts_created[i].bearer_level_qos.presence = 1;
            ogs_gtp2_build_bearer_qos(
                    &rsp->bearer_contexts_created[i].bearer_level_qos,
                    &bearer_qos, bearer_qos_buf[i], GTP2_BEARER_QOS_LEN);
        }

        /* Bearer Charging ID */
        rsp->bearer_contexts_created[i].charging_id.presence = 1;
        rsp->bearer_contexts_created[i].charging_id.u32 = sess->charging.id;

        /* Data Plane(UL) : SMF-S5U */
        memset(&pgw_s5u_teid[i], 0, sizeof(ogs_gtp2_f_teid_t));
        pgw_s5u_teid[i].teid = htobe32(bearer->pgw_s5u_teid);
        ogs_assert(bearer->pgw_s5u_addr || bearer->pgw_s5u_addr6);
        rv = ogs_gtp2_sockaddr_to_f_teid(
            bearer->pgw_s5u_addr, bearer->pgw_s5u_addr6,
            &pgw_s5u_teid[i], &pgw_s5u_len[i]);
        ogs_expect_or_return_val(rv == OGS_OK, NULL);

        switch (sess->gtp_rat_type) {
        case OGS_GTP2_RAT_TYPE_EUTRAN:
            pgw_s5u_teid[i].interface_type = OGS_GTP2_F_TEID_S5_S8_PGW_GTP_U;
            rsp->bearer_contexts_created[i].s5_s8_u_sgw_f_teid.presence = 1;
            rsp->bearer_contexts_created[i].s5_s8_u_sgw_f_teid.data =
                &pgw_s5u_teid[i];
            rsp->bearer_contexts_created[i].s5_s8_u_sgw_f_teid.len =
                pgw_s5u_len[i];
            break;
        case OGS_GTP2_RAT_TYPE_WLAN:
            pgw_s5u_teid[i].interface_type = OGS_GTP2_F_TEID_S2B_U_PGW_GTP_U;
            rsp->bearer_contexts_created[i].s12_rnc_f_teid.presence = 1;
            rsp->bearer_contexts_created[i].s12_rnc_f_teid.data =
                &pgw_s5u_teid[i];
            rsp->bearer_contexts_created[i].s12_rnc_f_teid.len =
                pgw_s5u_len[i];
            break;
        default:
            ogs_error("Unknown RAT Type [%d]", sess->gtp_rat_type);
            ogs_assert_if_reached();
        }

        i++;
    }
#endif

    gtp_message.h.type = type;
    return ogs_gtp2_build_msg(&gtp_message);
}

ogs_pkbuf_t *sgwc_s11_build_downlink_data_notification(
        uint8_t cause_value, sgwc_bearer_t *bearer)
{
    ogs_gtp2_message_t message;
    ogs_gtp2_downlink_data_notification_t *noti = NULL;
    ogs_gtp2_cause_t cause;
    ogs_gtp2_arp_t arp;
    sgwc_sess_t *sess = NULL;

    ogs_assert(bearer);
    sess = bearer->sess;
    ogs_assert(sess);

    /* Build downlink notification message */
    noti = &message.downlink_data_notification;
    memset(&message, 0, sizeof(ogs_gtp2_message_t));

    /*
     * TS29.274 8.4 Cause Value
     *
     * 0 : Reserved. Shall not be sent and
     *     if received the Cause shall be treated as an invalid IE
     */
    if (cause_value != OGS_GTP2_CAUSE_UNDEFINED_VALUE) {
        memset(&cause, 0, sizeof(cause));
        cause.value = cause_value;
        noti->cause.presence = 1;
        noti->cause.len = sizeof(cause);
        noti->cause.data = &cause;
    }

    noti->eps_bearer_id.presence = 1;
    noti->eps_bearer_id.u8 = bearer->ebi;

    memset(&arp, 0, sizeof(arp));
    arp.pre_emption_vulnerability =
        sess->session.qos.arp.pre_emption_vulnerability;
    arp.priority_level = sess->session.qos.arp.priority_level;
    arp.pre_emption_capability = sess->session.qos.arp.pre_emption_capability;

    noti->allocation_retention_priority.presence = 1;
    noti->allocation_retention_priority.data = &arp;
    noti->allocation_retention_priority.len = sizeof(arp);

    message.h.type = OGS_GTP2_DOWNLINK_DATA_NOTIFICATION_TYPE;
    return ogs_gtp2_build_msg(&message);
}
