<template>
    <div class="form-container">
      <h1 class="form-h1">Logging Out....</h1>
    </div>
</template>

<script>
  import Repositories from "../api/Repositories";
  import delay from "../utils/delay";

  export default {
    async created() {
      await Promise.all([
        Repositories.Users.logout(),
        delay()
      ]);
      await this.$router.push('/login?success=Successfully logged out');
    }
  }
</script>
